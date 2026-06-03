# T66 Desired Systems Restructure Report

Generated: 2026-05-30  
Operator: Codex  
Validator: Claude  
Scope: Read-only discovery and implementation implications report. No gameplay code, data tables, assets, config, or saves were changed.

## Task Contract

Working task:
Operator: Codex
Validator: Claude
Scope: Read-only report covering the requested weapon, idol, vendor/item, gambler, hero-selection, pet, smart-loot, and cleanup/deprecation changes. Mini/minigame systems are excluded by default and were not audited.
Stop condition: A comprehensive report exists under `Reports/AgentReviews/`, live repo evidence is cited, and Claude validation is complete or blocked.

## Counts Up Front

- Heroes: 12 rows in `Content\Data\Heroes.csv`.
- Weapons: 192 rows in `Content\Data\Weapons.csv`, currently 16 per hero.
- Weapon branches: 4 branch types, 48 rows each for AOE, Bounce, DOT, and Pierce.
- Weapon rarities: 4 rarities, 48 rows each for Black, Red, Yellow, and White.
- Idols: 12 rows in `Content\Data\Idols.csv`, currently 3 per category across AOE, Bounce, DOT, and Pierce.
- Items: 30 rows in `Content\Data\Items.csv`.
- Companions: 16 rows in `Content\Data\Companions.csv`.
- Stages: 20 rows in `Content\Data\Stages.csv`, 5 difficulties x 4 stages each and 5 themes x 4 stages each.
- Bosses: 23 rows in `Content\Data\Bosses.csv`.
- Enemies: 60 rows in `Content\Data\Enemies.csv`; all 60 currently have `StatusEffectOnHit=None`.
- NPCs: 3 rows in `Content\Data\NPCs.csv`.
- Combat VFX weapon bindings: 4 rows in `Content\Data\CombatVFXBindings.csv`, all currently for Hero 1 black-tier AOE/Pierce/Bounce/DOT weapon-base effects.

## Recap Of Requested Direction

The requested direction is to move away from broad per-hero weapon branching and toward tighter hero kits. Each hero should have one locked weapon type, four total weapons across the four stage/rarity moments, an ultimate, and a stat-growth distribution that defines level-up identity. The first five desired weapon-type assignments are Hero 1 AOE, Hero 2 Pierce, Hero 3 AOE, Hero 4 Bounce, and Hero 5 DOT.

Idols should be reorganized around four elements: Fire, Ice, Electricity, and Nature. Each element should have one idol per attack type: AOE, Pierce, Bounce, and DOT. The idol altar should show four cards for the current element, reroll should advance through the four elements, and the fifth reroll screen should show a single central "No Idol" option that gives Damage, Attack Speed, and Attack Scale instead of an idol.

Items and vendor flow should be redesigned. Items should have one stat line only, not one primary plus one secondary line. Some items should be primary-stat items, some should be secondary-stat items, and a new elemental-power family should add Fire Power, Ice Power, Electricity Power, and Nature Power. Vendor should show four cards, remove the bottom inventory strip, put reroll below the cards, and expose Buy Items, Sell Items, and Buy Back Items modes.

Mob Loot should be a new no-stat sellable item dropped by every killed mob. It should be the first card in the vendor sell panel, and difficulty should increase how much Mob Loot is dropped. A future pet system should let small cute captured bosses collect only Mob Loot automatically.

The gambler screen should become a simple four-game card screen rather than an NPC/vendor-like screen. Blackjack and rock-paper-scissors should be removed from the offered game set. The requested odds ladder is approximately 2x, 3x, 5x, and 10x, using quick-resolution games such as coin flip, three-cup ball guess, and a wheel-style 10x game.

Hero selection should remove the secondary video slot below the hero subtitle/stats area and replace it with weapon and ultimate sections. Those sections should be clickable and should open a video player above the stats panel. Placeholder media is acceptable at first, but the infrastructure should exist.

Pet selection should sit next to Choose Companion. Pets should have selection, union, and skins similar to companions. Bosses become capturable as tiny cute pets after being defeated, and pet union should improve how quickly the pet collects Mob Loot.

Status effects should be removed from idols if any current idol behavior or data path still uses them. Smart loot and smart vendor logic should bias loot bags and vendor stock toward the hero's build, current weapon type, stat identity, and purchased-item history.

## Current State Versus Desired State

| Area | Currently Exists | Partially There | Missing For Requested Direction |
|---|---|---|---|
| Hero weapon identity | Heroes already have `PrimaryCategory` and stat-growth fields. | Current first five hero categories do not match the requested first-five map. | Locked one-branch-per-hero weapon progression and final branch map for all 12 heroes. |
| Weapon data | 192 rows, 16 per hero, four branches x four rarities. | Four branch foundations and four VFX bindings exist for Hero 1 black-tier weapons. | 4 weapons total per hero, altar selection behavior for one locked branch, data-generation rewrite, migration/reuse mapping. |
| Ultimates | `UltimateType` enum and hero data field exist. | All current hero rows checked have `UltimateType=None`. | Real ultimate assignments, UI/media content, runtime behavior where absent. |
| Idol data | 12 category idols exist. | Each idol has category, rarity icons, behavior tuning, and runtime manager stock. | 16 element/type idols, element field, elemental altar paging, No Idol singleton, elemental power scaling hooks. |
| Item data | 30 item templates exist, with primary and secondary stat fields. | Rarity-agnostic item templates already support per-slot rarity and line rolls. | One-stat-line item model, elemental power stat enum/fields, four item tiers per elemental stat, Mob Loot item behavior. |
| Vendor | Shop, sell, buyback, inventory, reroll, pricing, and display modes exist. | Shop and buyback are currently five-slot systems; sell panel exists but is inventory-strip based. | Four-card layouts, button rename/modes, first-card Mob Loot sell stack, no bottom inventory, smart vendor weighting. |
| Gambler | Coin Flip, RPS, Blackjack, Lottery, Plinko, and Box Opening exist with anti-cheat records. | Some quick-game infrastructure and payout recording can be reused. | Four-card simplified screen, removal of vendor/stats/inventory/borrow UI, new 3x and likely 10x games, anti-cheat enum/backend updates. |
| Hero selection media | Hero selection has preview controller and clickable Ultimate/Passive preview handlers. | Existing preview controller already routes to `UT66FrontendVideoPlayer`; companion preview media exists too. | Weapon preview clip, ultimate/weapon card layout replacing lower slot, placeholder media routing above stats panel, Choose Pet button. |
| Pets | Boss kill history, companion selection, companion skins, and companion union systems exist. | Companion selection is a strong UI/persistence pattern to reuse. | Pet data table, pet selection screen, boss capture interaction, pet actor AI, Mob Loot vacuum behavior, pet union and skins. |
| Status effects | Status-effect enum and `StatusEffects.csv` exist; enemy rows are all `None`. | Existing roster already avoids production mob status-effect assignment. | Idol audit/removal of any status effect references in idol runtime/VFX/damage-source paths if present. |
| Smart loot/vendor | Shop has deterministic reroll and duplicate avoidance. | Current shop weighting only reduces repeat offers; loot bags choose random item IDs. | Build-aware weighting based on hero type, stat distribution, purchased item history, and active build state. |

## Domain Implications

### 1. Weapons And Hero Kits

Current implementation:
- `FHeroData` already carries hero identity, `PrimaryCategory`, `UltimateType`, base stats, and per-level gain ranges.
- `FWeaponData` already carries `HeroID`, `Rarity`, `Branch`, multipliers, and branch-specific tuning.
- `UT66WeaponManagerSubsystem::BuildWeaponOffers` currently builds four offers for a hero at a rarity by generating one weapon ID for each branch: Pierce, Bounce, AOE, and DOT.
- Weapon ID generation currently follows a hero/rarity/branch pattern such as `Hero_1_black_aoe`.
- `Scripts\SetupWeaponsDataTable.py` generates `Weapons.csv` from heroes and currently treats branches as a fixed four-branch set.

Requested shift:
- Each hero should have exactly four weapons total, not 16.
- Those four weapons should represent the four rarity/stage moments for that hero's locked branch.
- Weapon altar behavior should stop offering all four branch types for a given hero and instead follow the hero's locked weapon path.
- Hero kit becomes: locked weapon family, ultimate, and stat distribution.

Code/data implications:
- `Heroes.csv` needs a final locked attack category for every hero. The user provided the first five desired categories; the remaining seven still need a final product mapping.
- `Weapons.csv` should be regenerated or migrated from 192 rows to a target of 48 rows if all 12 heroes remain active: 12 heroes x 4 rarities.
- `UT66WeaponManagerSubsystem::BuildWeaponOffers` needs a new offer model. If the altar still shows four cards, those cards need a new meaning; if the altar now awards only the stage rarity for the locked branch, the UI should be simplified accordingly.
- `MakeWeaponID` can still work if the naming scheme remains `Hero_X_<rarity>_<branch>`, but the generator should only create rows for each hero's locked branch.
- `CombatVFXBindings.csv` and `SetupCombatVFXBindingsDataTable.py` currently hardcode Hero 1 black-tier branch IDs. Reusing Hero 1 pierce animation for Hero 2 black-tier Pierce will require repointing binding `SourceID`s and likely deciding whether VFX paths stay under Hero 1 folders or move to neutral/shared folders.
- Hero ultimate work needs assignment data first, then runtime behavior and UI/media preview paths. The enum exists, but current hero rows inspected still use `UltimateType=None`.

Reuse opportunity:
- The recent four-branch weapon infrastructure is directly reusable as attack-type foundation. The implementation change is mainly ownership and mapping: existing branch behaviors move from "every hero can choose every branch" to "each hero owns one branch across four rarity/stage moments."
- Existing Hero 1 branch effects can seed the new first wave. Example: current `Hero_1_black_pierce` can become the Hero 2 black Pierce foundation if the user approves that mapping.

### 2. Idols, Elements, And Idol Altar

Current implementation:
- `Content\Data\Idols.csv` has 12 idols: 3 AOE, 3 Bounce, 3 DOT, and 3 Pierce.
- `FIdolData` has category and behavior fields, but no explicit element field.
- `UT66IdolManagerSubsystem` has three equipped idol slots, max idol level 4, and a 12-slot stock model.
- `UT66IdolManagerSubsystem::GetAllIdolIDs` returns a fixed 12-idol list.
- `RerollIdolStock` currently fills stock from the full list rather than paging through elements.
- The idol altar UI already has reroll handling and card-description presentation, but its current structure is tied to existing stock/category presentation.

Requested shift:
- Create 16 functional idol identities: 4 elements x 4 attack types.
- Elements: Fire, Ice, Electricity, Nature.
- Types per element: AOE, Pierce, Bounce, DOT.
- Altar should show 4 cards for one element at a time.
- Reroll cycles element pages; after the fourth element, a fifth page shows only a central "No Idol" option.
- Cards should show idol name, image, and type text only; no rarity text below the name.
- No Idol grants Damage, Attack Speed, and Attack Scale instead of an idol.
- Status effects should be removed from idols if any are still present in idol behavior.

Code/data implications:
- `FIdolData` needs an element concept. This could be a new enum such as Fire/Ice/Electricity/Nature, a row field, or an interpreted naming convention, but a real field is cleaner for UI and smart-scaling.
- `Idols.csv` expands from 12 rows to 16 rows, unless the team keeps some current idols as aliases during migration.
- Current manager constant `IdolStockSlotCount=12` no longer matches the desired visible model. The future structure wants 4 visible offer cards per element page plus one special singleton page.
- `GetAllIdolIDs` should no longer be a fixed hardcoded 12-idol list if the table becomes authoritative.
- Reroll state needs to become page state: Fire -> Ice -> Electricity -> Nature -> No Idol, then probably repeat or close depending on design.
- Equipped idol state and save serialization need a migration decision for old 12-idol IDs.
- `No Idol` is not a normal idol in behavior; it is closer to a stat reward option. It can be represented as a special idol row only if every downstream consumer can handle it without expecting attack behavior.
- Elemental power stats must scale idol attacks by element only. That requires runtime damage/attack speed/attack scale paths to know the idol's element at damage time.

Reuse opportunity:
- The current 12 idols can seed the new 16-grid instead of being discarded immediately.
- The user-proposed reuse pattern is viable as a mapping exercise: existing idol attacks/animations can be reclassified into type slots for Fire first, then cloned/remapped for Ice, Electricity, and Nature as art/effect coverage grows.
- Current idol behavior fields are a foundation for attack category mechanics; the missing layer is element ownership and altar pagination.

### 3. Items, Vendor, Mob Loot, And Elemental Power

Current implementation:
- `FItemData` is built around one primary stat field and one secondary stat field.
- `FT66InventorySlot` stores item template ID, rarity, `Line1RolledValue`, secondary override, line 2 multiplier, and roll seed.
- Items are rarity-agnostic templates; runtime slots carry the item rarity.
- Current `Items.csv` has 30 item templates. Most items have both a primary stat type and a secondary stat type.
- `UT66RunStateSubsystem` has `ShopDisplaySlotCount=5` and `BuybackDisplaySlotCount=5`.
- Shop stock generation filters eligible item IDs, avoids duplicates, and applies a simple seen-count weighting.
- Current shop slot rarities are hardcoded to five visible slots: Black, Black, Black, Red, Yellow.
- Vendor UI builds shop slots, inventory strip, sell panel, buyback panel, reroll button, borrow/payback actions, and stats.
- Loot bags currently roll a rarity and then ask `UT66GameInstance::GetRandomItemIDForLootRarityFromStream` for an item ID.
- Player pickup consumes loot bags and adds the rolled item to inventory or applies the existing vendor token special case.

Requested shift:
- Vendor visible stock becomes four item cards.
- Bottom inventory strip is removed from vendor shop layout.
- Reroll button moves below the item cards inside the item-card panel.
- Buttons become Buy Items, Sell Items, and Buy Back Items.
- Sell Items shows four inventory cards, and reroll/pages through the next four.
- Mob Loot should always be first in the Sell Items panel, give no stats, and exist only to sell for gold.
- Items should have one stat line total. There can be primary-stat items and secondary-stat items, but not both on one item.
- Add four elemental power stats: Fire Power, Ice Power, Electricity Power, Nature Power.
- Elemental powers should appear at the bottom of the stat summary.
- There should be four items per elemental power stat: black, red, yellow, and white.
- Smart loot/vendor should bias offers toward the player's build.

Code/data implications:
- `ET66SecondaryStatType` currently has category stats such as AOE/Pierce/Bounce/DOT damage/speed/scale, but no Fire/Ice/Electricity/Nature Power values.
- `FT66HeroStatBonuses` currently has generic and category stats, but no element-specific power fields.
- Item data and UI formatting need a one-line item contract. This likely means either keeping `PrimaryStatType` and `SecondaryStatType` but enforcing only one populated field, or replacing them with a single stat descriptor field.
- Rarity currently belongs to inventory slots, not item rows. "Four items per elemental stat, a black/red/yellow/white" can either be implemented as one template per elemental stat with rarity instances, or as four explicit templates per stat. The current architecture favors one template with runtime rarity, but the user wording may prefer visible named item rows per rarity. This is a product/data decision before implementation.
- Shop and buyback constants should change from five to four, and all UI arrays/layout assumptions tied to those constants need to follow.
- Sell mode needs its own four-card page model, separate from the bottom inventory strip.
- Mob Loot likely needs either a stackable run-state currency-like count or a stackable inventory slot. Since it must always be first in Sell Items and gives no stats, a dedicated run-state count may be simpler for gameplay, while a synthetic first card can still make it look like an item in vendor UI.
- Difficulty-scaled Mob Loot drop amount needs a new table or formula. It should be tied to mob kill events, not loot bag pickup events.
- Pet collection needs Mob Loot pickups to exist in the world, or it needs a deferred "collectible loot count" generated near killed mobs. The UI requirement says item called Mob Loot; the pet requirement says collect it like a vacuum, so a world pickup actor is likely needed.

Reuse opportunity:
- Existing shop stock, inventory slot, buyback pool, sell value, and item reveal paths are reusable.
- Existing vendor token handling is a useful example of a no-normal-stat special item, but Mob Loot differs because it is frequent, stack-like, and sell-only.
- Existing category stat items provide a template for smart-loot matching by attack type.

### 4. Gambler Screen And Games

Current implementation:
- The gambler tab currently has pages for Dialogue, Casino, Coin Flip, Rock Paper Scissors, Black Jack, Lottery, Plinko, and Box Opening.
- The gambler UI currently includes stats, inventory, buyback, borrow/payback, game cards, and child game widgets.
- The backend anti-cheat run save enum currently records Coin Flip, Rock Paper Scissors, Black Jack, Lottery, Plinko, and Box Opening.
- Backend serialization converts gambler game summaries/events into strings such as `rps`, `blackjack`, `lottery`, `plinko`, and `box_opening`.
- Plinko and Box Opening already provide quick animated chance-game patterns with anti-cheat event recording.

Requested shift:
- No vendor button at top.
- No stats, inventory, borrow-money, or vendor-like economy panels.
- Show only four gamble game cards.
- Remove Blackjack and Rock Paper Scissors from the offered set.
- Use four quick games with clear odds and tiered payouts, roughly 2x, 3x, 5x, and 10x.

Suggested four-game set:
- Coin Flip, 2x: Existing concept and anti-cheat path can be reused.
- Three Cups, 3x: New simple game; player picks one of three cups, ball location resolves immediately after a short shuffle.
- Five Mark Pick, 5x: New simple 1-in-5 reveal game. Possible presentation: five sealed cards, one marked winner; player picks one and it flips immediately.
- Prize Wheel, 10x: New or adapted wheel/box-opening style game with ten equal segments and one winning segment; quick spin resolves to 10x or loss.

Alternative candidates:
- Keep Plinko as the 5x or 10x game only if the payout model is simplified; current Plinko includes multiple payout tiers, including very high and low multipliers.
- Keep Box Opening as a visual basis for the wheel/card reveal if it is renamed and payout odds are simplified.
- Lottery is already present but is slower and more complex than the requested quick card set.

Code/data implications:
- `EGamblerPage` and child game widget creation need to remove or hide RPS/Blackjack from the main experience and add new game pages/widgets for Three Cups, Five Mark Pick, and Prize Wheel if selected.
- `ET66AntiCheatGamblerGameType` needs new enum values and serializer mapping strings for any new games.
- Backend consumers and run-summary display paths need to understand the new game strings.
- Existing borrow/payback and buyback UI can remain in code if deprecated, but should be unreachable from the simplified screen unless the user chooses aggressive deletion.

### 5. Hero Selection And Preview Media

Current implementation:
- Hero selection already has a preview controller and `UT66FrontendVideoPlayer`.
- Current preview clip enum includes Overview, Ultimate, and Passive.
- Hero selection has click handlers for Ultimate and Passive preview.
- Companion selection also uses frontend video catalog/player infrastructure.
- Earlier UI documentation notes that a right-column video/poster slot exists below the hero subtitle, and the requested direction removes that lower slot.

Requested shift:
- Remove the lower video slot below the subtitle/stats area.
- Add Weapon and Ultimate sections below stats.
- Make Weapon and Ultimate sections clickable.
- Clicking either opens a video player above the stats panel.
- Placeholder media is acceptable initially; the infrastructure should be real.
- Add a Choose Pet button next to Choose Companion.

Code/data implications:
- Add `Weapon` as a hero preview clip type or equivalent preview target.
- Decide whether Ultimate should reuse the existing Ultimate preview handler or whether both Weapon and Ultimate should be routed through a new shared "kit preview" panel.
- Extend video catalog lookups to support hero weapon and hero ultimate clips, with placeholders as valid assets.
- Update hero selection layout, focus/navigation, UI metadata, and capture hooks. Future implementation is UI fidelity governed and should use the repo UI fidelity loop.
- Choose Pet needs a screen route and modal/screen enum. If pet screen does not exist yet, the button can be added only after the pet screen route is stubbed or explicitly hidden until available.

Reuse opportunity:
- Existing preview controller and video player infrastructure are the right base for the new clicked previews.
- Companion selection media and skin handling are a useful pattern for pet selection media later.

### 6. Pets, Boss Capture, Union, And Mob Loot Vacuum

Current implementation:
- There are 16 companions with companion selection, companion skins, companion union, and companion preview media.
- Profile save already tracks companion union and hero unity maps.
- Stage clear currently increments hero unity and companion union.
- Boss data exists with 23 boss rows, and profile save tracks lifetime bosses killed and lab-unlocked enemies/bosses.
- No obvious pet data table, pet selection screen, pet capture flow, or pet actor vacuum system was found in the audited main-run paths.

Requested shift:
- Add Choose Pet next to Choose Companion.
- After beating a stage boss, that boss becomes small/cute and interactable for capture as a pet.
- Captured pet collects Mob Loot only, not loot bags or other rewards.
- Pet has union and skins, with a selection screen similar to companion selection.
- Pet union improves movement speed/collection speed for Mob Loot quality of life.

Code/data implications:
- New pet data table likely needed. It can be derived from bosses, but a table gives control over display name, cute asset, unlock/capture state, skin set, movement/vacuum tuning, and preview media.
- Profile save needs pet ownership, equipped pet, pet skins, and pet union progression fields. Save migration must set safe defaults.
- Boss death flow needs a capture opportunity after stage boss defeat. This is separate from normal kill tracking and should not conflict with stage completion/descent flow.
- Runtime pet actor needs follow/collect behavior, target filtering for Mob Loot only, pickup consume logic, and fail-safe behavior when no Mob Loot exists.
- Pet selection can reuse companion selection layout patterns, but should be its own screen because pet role, data, and union stat are different.
- Mob Loot world pickup design should be settled before pet implementation; otherwise the pet has no concrete target to collect.

Reuse opportunity:
- Companion selection, companion skins, companion union, skin subsystem, and profile save migration patterns are the primary reuse surfaces.
- Boss roster can seed pet identities, but pet presentation should probably be data-driven rather than relying only on boss combat rows.

### 7. Status Effects

Current implementation:
- `ET66HeroStatusEffectType` exists with None, Burn, Chill, and Curse.
- `StatusEffects.csv` has 12 rows.
- `Enemies.csv` has 60 rows and all currently use `StatusEffectOnHit=None`.
- A pending data note says production mobs are not using status effects in the current roster path.
- `FIdolData` does not expose an obvious status-effect field in the data struct anchors inspected.

Requested shift:
- Remove status effects from idols if they have them.

Code/data implications:
- The first implementation step should be an idol-specific search for status-effect references in idol runtime damage, VFX, tooltip text, and damage-source tagging.
- If no idol runtime status effects exist, the change may be mostly data/text cleanup.
- Existing global status-effect definitions do not have to be deleted for this request unless the user chooses an aggressive cleanup phase; current production enemies already use `None`.

### 8. Smart Loot And Smart Vendor

Current implementation:
- Vendor stock generation already has a deterministic per-stage/per-reroll seed and avoids repeat items.
- Current weighting reduces repeated offers by item seen count.
- Loot bag item selection currently asks for a random live item template; it is not build-aware.
- Hero data and item data already expose attack category and stat types that can be used as weighting inputs.

Requested shift:
- Loot bags and vendor stock should favor the current build.
- Examples: AOE hero sees more AOE stat items; high-armor hero sees more armor items; buying build-specific items causes more of that kind to appear.

Code/data implications:
- Need a build-affinity model, likely in RunState or a dedicated loot/vendor weighting helper.
- Inputs should include selected hero, locked weapon branch, hero base/growth stat profile, owned/equipped items, purchased items, and possibly current stat totals.
- Outputs should be weights, not hard filters, unless the user wants deterministic narrow builds.
- Vendor and loot bag should share the same affinity logic so odds feel consistent.
- UI does not need to expose this directly unless future design wants tags or recommendation indicators.

### 9. Backend, Saves, And Online-Facing Data

Current implementation:
- Run summary submits to backend/leaderboard paths.
- Anti-cheat gambler summaries and events are saved and serialized into backend JSON.
- Profile save already tracks skins, union, achievements, boss kills, and lab unlocks.
- Run save stores inventory slots, gambler anti-cheat summaries/events, and run-state data.

Requested shift implications:
- New gambler games require anti-cheat enum additions, serializer strings, and backend compatibility decisions.
- Old game types such as RPS/Blackjack can be kept as legacy enum values for old saves even if hidden from UI.
- New pet ownership/union/skins require profile save migration.
- New item stat model and old inventory slots require run-save migration or compatibility conversion.
- Old weapon and idol IDs in existing saves need a migration rule, especially if rows are removed rather than deprecated.
- Elemental Power stats and No Idol stat rewards need to appear in run summaries/stat panels if they are player-facing.

### 10. Content Pipeline And Validation Implications

Current implementation:
- Weapon data has an owned generator/reload script: `Scripts\SetupWeaponsDataTable.py`.
- Item data has reload/import scripts: `Scripts\SetupItemsDataTable.py` and `Scripts\ImportItemSprites.py`.
- Combat roster/status tables have a reload script: `Scripts\SetupCombatRosterDataTables.py`.
- Combat VFX bindings have a reload script: `Scripts\SetupCombatVFXBindingsDataTable.py`.
- I did not find a standalone idol DataTable setup script during this pass; idol reload ownership should be identified before idol data implementation.

Implementation implications:
- Data CSV changes are not enough by themselves when runtime DataTables must be refreshed; the owning Unreal reload/import scripts or commandlets must run after data edits.
- UI changes to hero selection, vendor, idol altar, and gambler are visual/UI fidelity tasks and should use Unreal-owned screenshot/capture routes.
- Weapon/idol VFX reuse or remapping is process-governed visual/gameplay proof work. Future implementation phases should use the combat VFX process and multi-frame evidence where behavior is visual/temporal.
- Staged standalone verification is required after changes that affect playable standalone behavior.

## Suggested Agent Split

This split is a recommendation for planning the next implementation phase; it is not a requirement.

Phase 0: Data contract and migration map, one lead agent.
- Finalize hero-to-weapon-type map for all 12 heroes.
- Finalize weapon row naming and whether old rows become aliases or are deleted.
- Finalize idol element/type mapping, No Idol behavior, and old-idol migration.
- Finalize one-line item schema, elemental power formulas, Mob Loot representation, and gambler game set.
- Produce one shared schema packet before parallel implementation starts.

Workstream A: Weapons and hero kits.
- Owns hero category map, weapon data generator, weapon altar behavior, weapon VFX binding remap, and hero ultimate assignment scaffolding.
- Should coordinate with Workstream E for hero selection preview and with Workstream B for shared attack-category concepts.

Workstream B: Idols and elemental power.
- Owns idol data restructure, element/type model, idol altar paging, No Idol option, idol runtime scaling by element, and status-effect removal audit.
- Must coordinate with Workstream C because elemental power items feed idol behavior.

Workstream C: Items, vendor, Mob Loot, and smart vendor/loot weighting.
- Owns one-line item model, elemental power items, vendor four-card modes, Sell Items pagination, Mob Loot economy, and smart weighting shared by vendor/loot bags.
- Must coordinate with Workstream F because pets collect Mob Loot.

Workstream D: Gambler simplification.
- Owns four-card gambler screen, selected games, payout logic, anti-cheat enum/events, run serializer strings, and removal/hiding of old RPS/Blackjack/borrow/vendor-style UI paths.
- This stream is relatively separable once shared economy constraints such as wager limits are set.

Workstream E: Hero selection UI and media preview.
- Owns weapon/ultimate sections, clicked preview player, placeholder video routing, lower-slot removal, and Choose Pet button placement.
- Must coordinate with Workstream F for actual pet selection route availability.

Workstream F: Pets and boss capture.
- Owns pet data, profile persistence, pet selection screen, boss capture interaction, pet actor behavior, pet union, skins, and Mob Loot-only vacuum logic.
- Should start after Workstream C defines Mob Loot representation.

Integration/validation stream:
- Owns compile, DataTable reloads, save migration checks, screenshot captures, staged standalone refresh, and regression smoke matrix after the feature streams land.

Recommended parallelization:
- Start with Phase 0 as a single shared planning/data contract pass.
- Then split into 5 implementation chats: A, B, C, D, and E/F combined initially. Separate F into its own chat after Mob Loot is concretely defined.
- Keep one integration validator chat separate from feature implementation chats, because several systems cross through RunState, GameInstance, UI, and save/backend serialization.

## Suggested Cleanup, Deprecation, And Deletion Plan

This is only a suggested cleanup plan. The user may choose a more aggressive deletion strategy.

Conservative plan:
- Add the new schema and behavior while keeping old weapon, idol, item, and gambler IDs as deprecated compatibility aliases.
- Hide old content from UI and generation pools first.
- Add save migration paths for old weapon IDs, old idol IDs, old item stat lines, and old gambler game records.
- Keep old assets until reference audits and staged validation prove they are unreachable.
- Use this if preserving older saves and lowering implementation risk is more important than immediate cleanup.

Moderate plan:
- Remove obsolete CSV rows after migration rules and DataTables are updated.
- Keep unused assets for one release cycle while code and UI stop referencing them.
- Keep legacy enum values for save/backend compatibility, but prevent new runtime creation.
- Use this if the team wants the content tables to reflect the new design quickly while avoiding asset deletion surprises.

Aggressive plan:
- Delete old weapon rows, old idol rows, RPS/Blackjack visible UI, unused gambler child widgets, obsolete item templates, and unused assets after a reference audit.
- Remove or hard-disable old generation logic rather than leaving compatibility routes.
- Requires an explicit old-save policy. If old saves are allowed to break or reset, migration can be smaller; if not, this strategy still needs compatibility shims.
- Requires careful Mini-exclusion handling. Some shared sprite/assets notes mention Mini-inclusive audits before deletion, and Mini/minigame systems were not part of this pass.

Suggested default:
- Use the conservative plan for first implementation, then schedule a separate cleanup-only phase where the user decides how aggressive to be.

## Open Product Decisions Before Implementation

- Confirm locked weapon branch for Heroes 6-12.
- Confirm whether weapon altar still shows four cards or becomes one locked weapon pickup per stage/rarity.
- Confirm exact rarity/stage mapping: Stage 1 Black, Stage 2 Red, Stage 3 Yellow, Stage 4 White appears intended, but should be written as a data contract.
- Confirm whether the current 192 weapon rows should be migrated to 48 rows immediately or deprecated first.
- Confirm exact idol element order for reroll paging and whether page five loops back to Fire after choosing/rerolling No Idol.
- Confirm No Idol stat values, scaling, rarity/tier handling, and whether it occupies an equipped idol slot.
- Confirm final mapping from existing idol attacks/animations to Fire/Ice/Electricity/Nature x AOE/Pierce/Bounce/DOT.
- Confirm whether elemental power is a percentage multiplier, flat value, or separate coefficient for damage, attack speed, and attack scale.
- Confirm whether elemental power appears only in stat summary or also in item cards/tooltips/run summary.
- Confirm whether Mob Loot is a world pickup actor, a stackable inventory item, or a run-state currency displayed as a sell card.
- Confirm Mob Loot drop amount by difficulty and whether bosses/specials drop different amounts.
- Confirm final four gambler games, wager limits, payout rounding, and loss/win presentation.
- Confirm pet capture rules: every stage boss, first-time only, chance-based or guaranteed, one active pet or multiple active pets, and capture timing relative to descent/stage clear.
- Confirm old-save policy for weapon/idol/item/gambler changes.

## Evidence Appendix

Process and routing:
- `C:\UE\T66\.t66\operator-state.json`: Operator is Codex and Validator is Claude.
- `C:\UE\T66\Reports\AGENTS.md`: report artifacts belong under `Reports\AgentReviews`.
- `C:\UE\T66\AGENTS.md`: Mini/minigame scope excluded by default; no native goal tools; Operator/Validator stack required for substantive results.

Project structure:
- `C:\UE\T66\T66.uproject:3`: engine association is 5.7.
- `C:\UE\T66\T66.uproject:8`: main runtime module is `T66`.
- `C:\UE\T66\T66.uproject:13`: `T66Mini` module exists, but Mini/minigame scope was excluded from this pass.
- `C:\UE\T66\T66.uproject:47`, `:61`, `:77`: PythonScriptPlugin, OnlineSubsystemSteam, and ElectraPlayer are enabled.

Data anchors:
- `C:\UE\T66\Content\Data\Heroes.csv`: 12 hero rows.
- `C:\UE\T66\Content\Data\Weapons.csv`: 192 weapon rows; 16 per hero.
- `C:\UE\T66\Content\Data\Idols.csv`: 12 idol rows.
- `C:\UE\T66\Content\Data\Items.csv`: 30 item rows.
- `C:\UE\T66\Content\Data\Companions.csv`: 16 companion rows.
- `C:\UE\T66\Content\Data\Stages.csv`: 20 stage rows; 5 difficulties x 4 stages, 5 themes x 4 stages.
- `C:\UE\T66\Content\Data\Bosses.csv`: 23 boss rows.
- `C:\UE\T66\Content\Data\Enemies.csv`: 60 enemy rows; all `StatusEffectOnHit=None`.
- `C:\UE\T66\Content\Data\CombatVFXBindings.csv`: 4 Hero 1 black-tier weapon VFX binding rows.

Code anchors:
- `C:\UE\T66\Source\T66\Data\T66DataTypes.h:21`: attack categories enum.
- `C:\UE\T66\Source\T66\Data\T66DataTypes.h:90`: weapon rarity enum.
- `C:\UE\T66\Source\T66\Data\T66DataTypes.h:100`: ultimate type enum.
- `C:\UE\T66\Source\T66\Data\T66DataTypes.h:151`: hero data struct.
- `C:\UE\T66\Source\T66\Data\T66DataTypes.h:463`: weapon data struct.
- `C:\UE\T66\Source\T66\Data\T66DataTypes.h:654`: hero stat bonus struct.
- `C:\UE\T66\Source\T66\Data\T66DataTypes.h:848`: primary hero stat enum.
- `C:\UE\T66\Source\T66\Data\T66DataTypes.h:867`: secondary stat enum.
- `C:\UE\T66\Source\T66\Data\T66DataTypes.h:974`: status effect enum.
- `C:\UE\T66\Source\T66\Data\T66DataTypes.h:995`: item data struct.
- `C:\UE\T66\Source\T66\Data\T66DataTypes.h:1594`: idol data struct.
- `C:\UE\T66\Source\T66\Core\T66WeaponManagerSubsystem.cpp:52`: current weapon offer builder.
- `C:\UE\T66\Source\T66\Core\T66WeaponManagerSubsystem.cpp:103`: weapon ID builder.
- `C:\UE\T66\Source\T66\Core\T66IdolManagerSubsystem.h:22-24`: idol equipped slots, max level, and 12-slot stock constants.
- `C:\UE\T66\Source\T66\Core\T66IdolManagerSubsystem.cpp:136`: fixed idol ID list.
- `C:\UE\T66\Source\T66\Core\T66IdolManagerSubsystem.cpp:277`: idol stock reroll.
- `C:\UE\T66\Source\T66\Core\T66RunStateSubsystem.h:115-116`: five shop and buyback display slot constants.
- `C:\UE\T66\Source\T66\Core\RunState\T66RunStateSubsystem_EconomyInventory.cpp:125`: current five-slot shop rarity pattern.
- `C:\UE\T66\Source\T66\Core\RunState\T66RunStateSubsystem_EconomyInventory.cpp:197`: shop reroll entry point.
- `C:\UE\T66\Source\T66\Core\RunState\T66RunStateSubsystem_EconomyInventory.cpp:352`: buyback display generation uses display slot count.
- `C:\UE\T66\Source\T66\Core\T66GameInstance.cpp:839`: random item ID selection for loot rarity.
- `C:\UE\T66\Source\T66\Gameplay\T66MobBase.cpp:562`: current mob death path adds hero XP.
- `C:\UE\T66\Source\T66\Core\RunState\T66RunStateSubsystem_Combat.cpp:442`: enemy-killed notification entry point.
- `C:\UE\T66\Source\T66\Gameplay\GameMode\T66GameMode_WorldInteractables.cpp:925`: loot bag configuration.
- `C:\UE\T66\Source\T66\Gameplay\T66PlayerController_Combat.cpp:1420`: loot bag pickup consumption path.
- `C:\UE\T66\Source\T66\Core\T66RunSaveGame.h:207-214`: current gambler anti-cheat game enum.
- `C:\UE\T66\Source\T66\Core\Backend\T66BackendRunSerializer.cpp:346-388`: gambler summaries/events backend serialization.
- `C:\UE\T66\Source\T66\UI\Screens\HeroSelection\T66HeroSelectionPreviewController.h:22-26`: hero preview clip enum.
- `C:\UE\T66\Source\T66\UI\Screens\HeroSelection\T66HeroSelectionPreviewController.cpp:187-196`: hero selection preview video resolve/open path.
- `C:\UE\T66\Source\T66\UI\Screens\T66HeroSelectionScreen.h:287-288`: ultimate/passive preview click handlers.
- `C:\UE\T66\Source\T66\UI\Screens\T66HeroSelectionScreen.cpp:709-717`: choose companion route.
- `C:\UE\T66\Source\T66\Core\T66ProfileSaveGame.h:139-150`: companion union and hero unity profile maps.
- `C:\UE\T66\Source\T66\Gameplay\T66StageGate.cpp:150-160`: stage clear increments hero unity and companion union.
- `C:\UE\T66\Source\T66\Core\T66AchievementsSubsystem.cpp:1325-1371`: companion union read/progress methods.
- `C:\UE\T66\Scripts\SetupWeaponsDataTable.py:2`: weapon CSV/DataTable generation script.
- `C:\UE\T66\Scripts\SetupItemsDataTable.py:2`: item DataTable reload script.
- `C:\UE\T66\Scripts\SetupCombatRosterDataTables.py:30`: status effects DataTable reload target.
- `C:\UE\T66\Scripts\SetupCombatVFXBindingsDataTable.py:37-91`: hardcoded Hero 1 branch VFX binding seed rows.

## Verification Performed

- Read root and folder process instructions relevant to Reports, UI, Gameplay, and data/report routing.
- Read `.t66\operator-state.json` and confirmed Codex operator / Claude validator.
- Counted live CSV rows for Heroes, Weapons, Idols, Items, Companions, Stages, Bosses, Enemies, NPCs, StatusEffects, and CombatVFXBindings.
- Performed targeted source searches for weapon offers, idol stock, item/vendor inventory, gambler games/anti-cheat, hero selection preview media, companion union/skins, loot bag pickup, mob kill events, data-table reload scripts, and backend serialization.
- No compile, editor launch, data reload, screenshot capture, or staged standalone run was performed because this request is a read-only planning/report pass.
