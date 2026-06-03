Verdict: REVISE

## Blockers

- **Execute-on-crit mechanic is invented, not user-approved.** The user said "Execute replacing secondary Accuracy" but never specified mechanics. The plan unilaterally chose: roll Execute after a successful crit, OHKO normals/mobs, exclude bosses. That is a gameplay design decision, not an implementation detail — it must be confirmed by the user before code lands. Other plausible designs (independent roll outside crit, percent-HP execute threshold, crit-damage multiplier, etc.) are not even discussed.
- **Legacy item save migration is hand-waved.** Plan says "legacy aliases must keep them working" but does not specify how previously-owned `Item_Accuracy`, `Item_TreasureChest`, `Item_GamblersToken`, `Item_Cheating`, `Item_Stealing` are surfaced in an existing save. Are they remapped to the new IDs, hidden, dropped, refunded? Without an explicit mapping rule this risks orphaning owned items or duplicating them on resolve. Needs to be specified before implementation.

## Major Issues

- **Display-name updates are not enumerated.** User said "Chance suffix display names where requested" but the plan never lists which display names change in `T66DataTypes.h`. Live evidence shows current strings `Accuracy`, `Dodge`, `Damage Reflection`, `Taunt`, `Crush`, `Assassinate`. The plan must spell out the exact new display names per row before Codex executes, otherwise the deliverable is undefined.
- **"Counter Chance" vs "Counter Attack" naming conflict.** User's evasion order says "Counter Chance" but also said "Other names can remain as they are." The plan's CSV ID is `Item_CounterAttack` but the order line says Counter Chance. Resolve before implementation: is the display name being changed to "Counter Chance" or staying "Counter Attack"?
- **Headshot/Accuracy refactor scope is buried.** "Change the old head-targeting accuracy getter so it no longer depends on an item-facing Accuracy secondary. It should use hero base accuracy plus the Headshot passive" — this is a real behavior change to targeting, not a cosmetic rename. It deserves explicit design statement and verification beyond a startup smoke. Confirm Headshot passive currently exists and exposes the value the new getter expects.
- **Pending issues content not surfaced.** Packet lists five `pending_issues_*.md` files as read but does not state whether any item there conflicts with or already covers this taxonomy work. Per AGENTS.md the agent must reconcile pending issues; treat that as a precondition.

## Minor Issues

- **Verification smoke is thin for a gameplay-affecting change.** "Smoke the staged exe to at least menu/log startup" does not exercise Execute-on-crit, Loot* secondaries, Vendor Token text path, or save load with a legacy item. Add staged-standalone steps that actually trigger each affected system, or explicitly say staged smoke is preflight only and the user will do gameplay validation.
- **Cheating/Stealing disposition is ambiguous.** Plan says "Omit `Item_Cheating` and `Item_Stealing` from active CSV" — does that mean rows deleted, or kept as inactive rows? State which, because resolver behavior differs.
- **Placeholder icon reuse for `Item_LootBag` / `Item_LootWheel` not pinned.** Plan says "existing placeholder item icon references from the current item sprite set" without naming the references. Specify the exact sprite paths to be reused so the CSV row is deterministic.
- **`MASTER_PLAYER_EXPERIENCE.md`, `MASTER_MOVEMENT.md`, etc. not checked.** Docs section lists STATS and COMBAT only. Confirm no other master doc enumerates the old order or Gambler Token.
- **Execute path interaction with idol `Execute` helper not stated.** Live evidence flags an existing non-item `Execute` damage event in `T66CombatComponent.cpp`. Plan should explicitly state whether the new item Execute reuses, parallels, or stays disjoint from that helper to avoid two semantically different "Execute" effects coexisting silently.

## Clarifying Questions

1. Execute Chance mechanic: confirm "roll after crit; OHKO on success; bosses immune"? Or specify the actual intended formula.
2. Counter row display name: "Counter Attack" (kept) or "Counter Chance" (new)?
3. Legacy owned items in existing saves — preferred behavior: silent remap to new ID, hide, drop, or refund?
4. `Item_Cheating` / `Item_Stealing`: delete rows entirely, or retain as inactive/deprecated rows for save compatibility?
5. Should Vendor Token's internal save field/key be renamed in a future pass, or is the long-term plan to keep `GamblersToken` as the persisted name forever?

## Required Verification

- C++ Development build of `T66Editor` after enum additions and secondary stat switch updates.
- `Scripts/SetupItemsDataTable.py` reload of `/Game/Data/DT_Items`, with confirmation that the resulting asset matches new active row order.
- Narrow greps: (a) no player-facing `Gambler`, (b) CSV active rows match the documented order, (c) old `Item_Accuracy` / `Item_EvasionChance` row IDs no longer appear as active.
- Staged standalone build via `Scripts\StageStandaloneBuild.ps1`.
- Load a save containing at least one legacy item ID (e.g., `Item_GamblersToken`, `Item_TreasureChest`) and verify the documented migration outcome — not just a clean menu boot.
- A gameplay smoke that actually triggers a crit while holding `Item_ExecuteChance` to confirm the new OHKO path fires for a mob and does not fire for a boss. If that is out of scope for Codex, state so explicitly and gate it to user verification.
- Confirm `MASTER_STATS.md` is updated per the `Gameplay/Stats/MASTER_STATS.md` "update after material changes" rule, and that pending_issues docs are reconciled.

## Rationale

The data reordering, Vendor Token rename, deprecate-but-keep-parsable enum strategy, and reuse-of-existing-icons posture all look sound and consistent with AGENTS.md (data-first, no Mini scope, Claude review pre-implementation). What blocks approval is that the plan crosses from data/taxonomy into inventing a real combat mechanic (Execute-on-crit OHKO) and a targeting refactor (Accuracy getter dropping the item dependency) without an explicit user sign-off on either design, and it leaves legacy save migration and several display-name choices undefined. These are not implementation nits — they change behavior and player save outcomes. Tighten the design decisions and naming table, then this is greenlightable.

