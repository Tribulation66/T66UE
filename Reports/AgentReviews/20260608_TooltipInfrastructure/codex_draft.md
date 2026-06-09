# Tooltip Infrastructure Draft

## Task Contract

Working task:
Operator: Codex
Validator: Claude
Scope: design a broad tooltip infrastructure strategy and produce an extensive tooltip coverage list across UI/gameplay/economy/gambler/powerup surfaces; no product code edits until rollout scope is accepted.
Stop condition: Claude and Codex produce repo-grounded infrastructure recommendations, Codex synthesizes the best plan, and the user receives the coverage list plus concrete implementation phases.

## Live Repo Findings

- UI already has scattered tooltip implementations:
  - `Source/T66/UI/T66StatsPanelSlate.cpp` has local `MakeT66Tooltip(...)` and multiple stat/ability tooltip callers.
  - `Source/T66/UI/HUD/T66GameplayHUDWidget_Private.h` has local `CreateCustomTooltip(...)` and `CreateRichTooltip(...)`.
  - `Source/T66/UI/Style/T66FlatStyle.cpp` has `MakeFlatTooltipContent(...)`, `MakeFlatTooltipIcon(...)`, and `MakeFlatTabButton(...)` info icon tooltip support.
  - Several screens use `SetToolTipText`, `.ToolTipText`, or custom `SToolTip` directly.
- UI already has strong tagging and audit infrastructure:
  - `FT66FlatWidgetMetadata` carries tag, intended role/state, click handler, toggle group, label flag, and hover-capable flag.
  - `T66WidgetTreeWalker` exports `hover_capable`, `has_click_handler`, `toggle_group`, and intended role/state into UI dumps.
  - UI checklists already identify many `hover_capable=true` controls across main menu, settings, hero selection, achievements, gambler, casino, altars, pause, and modal screens.
- Localization and data already cover much of the content:
  - `UT66LocalizationSubsystem` exposes primary stat descriptions, secondary stat descriptions, passive/ultimate descriptions, item display names, rarity names, and idol tooltips.
  - `FItemData` and `FT66InventorySlot` carry primary stat line, secondary stat type, rarity, line 1 roll, line 2 multiplier, price scaling, and icon selection.
  - `T66ItemCardTextUtils` already formats item card description lines from item data.
  - `T66WidgetGameRegistry` owns casino game descriptors and text for Coin Flip, Guess the Cup, Pick the Stick, and Find the Joker.
- Existing pending issue: frontend screens lack a central controller focus contract. Tooltip payloads should be reusable for controller focus/detail help, but the first rollout can be mouse/Slate hover.
- Archived mods/RetroFX checklist rows still exist as a pending documentation issue, so new tooltip coverage should not invest in archived RetroFX controls except to keep existing compatibility screens from crashing.

## Recommended Architecture

Build a central tooltip contract rather than more per-widget strings.

1. `FT66TooltipPayload`
   - Fields: `TooltipId`, `Kind`, `Title`, `Body`, `Subtitle`, `Rows`, `Warnings`, `SourceTag`, `EntityId`, `Rarity`, `bPlayerSpecific`, `bDynamic`, optional min width/wrap width.
   - Kinds: Action, Stat, Item, Weapon, Idol, PowerUp, TemporaryBuff, VendorOffer, GamblerGame, Economy, Setting, Leaderboard, Achievement, Party, Status, WorldInteractable, Warning.

2. Shared Slate presentation helper
   - Promote the styled local helpers into one UI-owned helper, likely `UI/T66TooltipSlate.h/.cpp` or an `FT66FlatStyle` extension.
   - Expose `MakeTooltip(const FT66TooltipPayload&)`, `SetTooltipPayload(SWidget, Payload)`, and `MakeInfoIcon(TooltipId/Payload)`.
   - Use Slate-native `SToolTip` and existing flat panels so this stays compatible with current Slate UI.

3. Content and resolver layer
   - Static copy remains localization-backed.
   - Dynamic content comes from resolvers/providers:
     - stat resolver: primary/secondary stat names/descriptions/formula notes.
     - item resolver: item name, rarity, line 1, line 2, price/sell value, stack count.
     - powerup resolver: permanent vs single-use, owned/equipped state, cost, duration/slot rules.
     - vendor resolver: buy/sell/steal/reroll, buyback, inventory capacity, discount/sell rules.
     - gambler resolver: game rules, wager, payout, current gold, double-down/lock state, luck rescue chance if surfaced.
     - world interactable resolver: crate, loot wheel, idol/weapon altar, collector/lab, cowardice gate.
   - Do not hardcode live odds, prices, or stat magnitudes into tooltip text; derive from current data/subsystems.

4. Metadata and verification
   - Extend `FT66FlatWidgetMetadata` with optional `TooltipId`, `TooltipKind`, and `bTooltipRequired`.
   - Extend widget dumps with `has_tooltip`, `tooltip_id`, `tooltip_kind`, and `tooltip_required`.
   - Add a warning-only audit first: hover-capable/clickable tagged widgets without a tooltip are listed by screen.
   - Later, add commandlet/automation to hover selected tags and verify visible tooltip capture.

5. Rules
   - Every interactive element with an unclear consequence gets a tooltip.
   - Every stat, item stat line, powerup, temporary buff, rarity, currency, cost, chance, lockout, leaderboard/account status, or run-count consequence gets a tooltip.
   - Decorative art, plain repeated labels, body copy that already explains itself, and archived controls are exempt unless they are interactive or encode a hidden rule.
   - Tooltip content should explain consequence, duration, source, formula, lock state, cost, or why a button is disabled. It should not merely repeat the visible label.

## Coverage List

- Global: Back, close, quit, save and quit, reset, confirm/cancel, disabled buttons, selected tabs, info icons, pagination, filters, search, favorites, language/account/profile actions, ticket/currency badges.
- Settings: gameplay reset do-not-show warnings, HUD toggles, audio sliders, controls/rebinding rows, graphics presets, media viewer options, crash/report toggles, all settings tabs. RetroFX should be excluded from new work if archived.
- Main menu/topbar: Enter Tribulation, load/save, profile, party, account status, achievements, leaderboards, powerups, quit, tickets, active warnings/status.
- Hero selection: hero cards, locked/unlocked state, party slots/readiness, solo/party mode, passive/ultimate, primary category, base stats, skins, map/theme, enter readiness, run-not-count warning reasons.
- Companions/pets: companion grid slots, recruit/unlock state, companion role/benefit, pet selection slots, no-companion slot, boss/source unlock clues.
- Stats: primary stats, secondary stats, category stats, derived armor/evasion values, elemental power, rarity/line multiplier, deprecated compatibility stat lines only when they still appear.
- Items/vendor: shop item name/icon/panel, rarity, line 1 stat roll, line 2 stat effect, buy price, sell price, steal button and timing prompt, buyback card, empty inventory slot, inventory count, mob loot stack, reroll, vendor token/discount rules, insufficient gold/capacity states.
- Powerups: permanent stat fill steps, diploma/relic cards, crystal/coupon currencies, single-use buff cards, owned/equipped state, selected drug slots, blocked/demo/coming-soon states, maxed state, costs.
- Gambler: casino open/back, wager spinbox, bet, double down, cash out/close, locked-game state, current gold, each game card, child-game choices, payout multipliers, base odds, luck rescue/assist rule if exposed, blocked demo gate.
- World overlays/interactables: crate rewards, loot wheel spin/reward/boost result, idol altar slot/offer/select/max/no-empty-slot, weapon altar offer/select, collector/lab turn-in, cowardice gate choice, casino NPC, recruitable companion, tutorial guide prompts.
- HUD/in-run: hearts/health, stage, timer, gold, debt, score, XP/level ring, weapon/passive/ultimate, idol slots, inventory slots, temporary buff slots, cooldowns/charges, minimap/full map controls, enemy lock, notifications/toasts.
- Run summary/history/leaderboards: score breakdown, leaderboard eligibility, rank/filters, personal best, party member names, run owner, inventory/idols/buffs at end, achievements earned, run-not-count reasons.
- Account/community/moderation: account status and appeal status, suspended warning, community submission status, challenge/mod status if still visible, backend/offline state, unsupported leaderboard upload state.
- Modals/popups: party invite accept/reject, quit confirmation choices, save preview/load slot metadata, report bug fields/send button, consolidated run-will-not-count popup reason and do-not-show checkbox.

## Rollout

Phase 1: core helper/payload/metadata/dump fields plus a narrow pilot on stats, HUD inventory/idol slots, and flat info icons.
Phase 2: item/powerup resolvers and vendor card tooltips.
Phase 3: gambler/game resolver and economy state tooltips.
Phase 4: hero selection, party, run eligibility warnings, settings, account/leaderboard.
Phase 5: world overlays, remaining HUD, checklist regeneration, and tooltip coverage audit gate.

## Risks

- A raw `SetToolTipText` expansion will become impossible to maintain. Use payloads and resolvers.
- Controller support is not solved by mouse hover. Payloads should be input-agnostic even if initial presentation is Slate hover.
- Tooltip text can become stale if odds, prices, and stat formulas are copied instead of derived.
- Layout can regress if tooltips are too wide or use unwrapped long item names. Apply fixed width/wrap and UI dump checks.

## Draft Final Recommendation

Build this as a UI-owned tooltip contract on top of existing Slate tooltip, flat style, localization, and metadata systems. The first implementation should be the core payload/helper plus dump metadata and a pilot covering stat/item/powerup/vendor/gambler examples. Then convert screen-by-screen using the hover-capable checklist rows as the coverage ledger.
