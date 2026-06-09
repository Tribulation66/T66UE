You are Claude cross-reviewing a Codex draft for the T66 Unreal project.

Rules:
- Include a clear Result: OK or Result: NEEDS_USER line near the top.
- Prefer starting with the result line, but the parser will also accept a clear
  result line or unambiguous OK / needs-user meaning elsewhere in the response.
- Do not edit files.
- Do not run mutating commands.
- Treat Codex as the Operator/final router and you as the Validator.
- Compare the original prompt, Codex draft, and your independent answer when present.
- Look specifically for mistakes, missed constraints, risky assumptions, weak evidence, scope problems, and unclear wording.
- Patch the answer text when the fix is straightforward.
- Return concrete issues when Codex needs to inspect, edit, verify, or ask the user before answering.
- Ask a user question only when the user is the only person who can decide the next path.
- Keep the review concise and practical. Do not create packet-completeness ceremony or hard review-depth categories.

Your result should be one of these two lines:
Result: OK
Result: NEEDS_USER

After that result line, return a concise Markdown review with exactly these headings:
Summary
Suggested Answer Patch
Issues To Fix
Question For User
Evidence Or Verification Gaps
Notes

Result meanings:
- OK: the models can handle the prompt internally. You may still list corrections, evidence gaps, or wording patches for Codex to handle before answering.
- NEEDS_USER: the user's attention is required because only the user can decide, approve, unblock a missing prerequisite, resolve an unavailable required tool, or change the scope.

Do not use NEEDS_USER for ordinary mistakes or missing edits that Codex can fix. List those inside the review body and keep the result OK.

Review scope:
- Original prompt path: C:\UE\T66\Reports\AgentReviews\20260608_TooltipInfrastructure\original_prompt.md
- Codex draft path: C:\UE\T66\Reports\AgentReviews\20260608_TooltipInfrastructure\codex_draft.md
- Independent answer path: C:\UE\T66\Reports\AgentReviews\20260608_TooltipInfrastructure\20260608T040534-IndependentAnswer-pass1\claude_review_pass1.md
- Output scope: targeted cross-review and answer patch only.

<original_prompt>
Original user request:

Ok the next thing I want you to do is to build a tool tip system, what I basically want to do is if your mouse hovers over anything, it should have a tool tip explaining it, like permenant or temporary powerups, if I tooltip over a stat of an item in the vendor, over a game, in the gambler, this is going to be very extensive, so go ahead and think with claude what the best infrastructure for this would be. And give me an extensive list of what could have a tooltip. Under this philosophy

Task contract:

Working task:
Operator: Codex
Validator: Claude
Scope: design the tooltip infrastructure strategy and produce an extensive tooltip coverage list across UI/gameplay/economy/gambler/powerup surfaces; no code edits until architecture and rollout scope are accepted.
Stop condition: Claude and Codex produce repo-grounded infrastructure recommendations, Codex synthesizes the best plan, and the user receives the coverage list plus concrete implementation phases.

Relevant repo rules:
- Do not use native goal tools.
- Follow `AGENTS.md` and `OPERATOR_VALIDATOR_PROTOCOL.md`.
- Codex is Operator and Claude is Validator according to `.t66/operator-state.json`.
- UI owns frontend Slate tooltips and verification. Read `UI/UI_AGENTS.md` and `UI/Instructions/UI_LAYOUT_AND_SIZING_INSTRUCTIONS.md`.
- Gameplay/economy/gambler content must be validated against live source/data before implementation.
- This is a planning/infrastructure pass only; no mutation beyond AgentReview artifacts.

</original_prompt>

<validator_independent_answer>
I have enough to give a grounded independent answer.

Result: OK

## Independent Answer

This is a planning/infrastructure pass the Operator/Validator pair can handle internally — no user decision is blocking. The key independent finding is that **T66 already has the bones of a tooltip system**, so the recommendation is "promote and extend," not "build from scratch."

**Existing infrastructure to build on:**
- `MakeT66Tooltip(Title, Description, FontSizeAdjustment)` in `Source/T66/UI/T66StatsPanelSlate.cpp:39` already produces a styled two-line (bold title + wrapped body) `SToolTip` using `FT66FlatStyle`. It is currently a `static` local — **promote it to a shared helper** (e.g. `T66ScreenSlateHelpers` or a new `T66TooltipHelpers.h`) so every surface uses one styling source.
- `T66LocalizationSubsystem` already exposes tooltip *content* getters: `GetText_PrimaryStatTooltip`, secondary-stat, passive, ultimate, and `GetText_IdolTooltip(FName)` (`T66LocalizationSubsystem.h:584-739`). This is the right home for tooltip copy — **data/loc-driven, keyed by ID**, not hardcoded per widget.

**Recommended architecture (3 layers):**
1. **Presentation** — one shared `MakeT66Tooltip` factory (Slate-native `SToolTip` / `.ToolTip()` / `SetToolTipText`). UI owns this per repo rules.
2. **Content registry** — extend the localization subsystem with `GetText_*Tooltip(ID)` getters per domain (stats, items, powerups, vendor actions, gambler odds, idols/altars, currencies). Copy lives here and is localizable.
3. **Binding** — each widget calls the factory + registry by the entity's existing ID/enum. No bespoke tooltip logic per widget.

**Extensive coverage list (surfaces confirmed to exist in `Source/T66/UI`):**
- **Stats/items**: primary stats, secondary/derived stats, passive & ultimate abilities, item card lines (`T66StatsPanelSlate`, `T66ItemCardTextUtils`) — already partially wired.
- **Powerups**: permanent vs temporary buffs (`T66PowerUpScreen`, `T66TemporaryBuffUIUtils`) — distinguish duration/stacking in copy.
- **Casino / economy**: Vendor tab (`T66CasinoVendorTabWidget`), Alchemy tab, currencies/prices, sell/buy actions (`T66CasinoOverlayWidget`).
- **Gambler**: gambler tab (`T66CasinoGamblerTabWidget`) and minigames — CoinFlip, GuessCup, StickPick, FindJoker (`Source/T66/UI/Gambler/*`). Rules, odds, payouts, stakes.
- **Altars/overlays**: Idol altar, Weapon altar, Collector, Crate, Lab, Loot wheel.
- **HUD/in-run**: inventory inspect, full map, buff icons, enemy lock (`T66GameplayHUDWidget_*`, `T66EnemyLockWidget`).
- **Selection screens**: hero/companion/pet selection, achievements, challenges, daily climb.
- **Meta/account**: account status, settings options, run summary, leaderboard panel.

**Phasing (low-risk first):**
- Phase 0: promote the shared helper + define the registry getter convention.
- Phase 1: stats/items/powerups (content getters already exist — highest leverage, lowest risk).
- Phase 2: casino vendor + gambler (requires data validation — see caveat).
- Phase 3: altars/overlays/HUD.
- Phase 4: selection + meta screens.

## Evidence Checked
- `.t66/operator-state.json` — Codex=Operator, Claude=Validator confirmed.
- `Reports/AgentReviews/20260608_TooltipInfrastructure/original_prompt.md` — task contract.
- `Source/T66/UI/T66StatsPanelSlate.cpp:39-64` — existing `MakeT66Tooltip` `SToolTip` factory.
- `Source/T66/Core/T66LocalizationSubsystem.h:576-739` — existing tooltip content getters.
- `Glob Source/T66/UI/**/*.h` + grep for Vendor/Gambler/Powerup — confirmed the surface inventory (Casino tabs, Gambler minigames, altars, overlays, HUD, selection screens).

## Questions Or Blockers
None requiring the user. This is planning-only and the architecture/coverage are repo-grounded. The user's eventual approval of *rollout scope* is the natural next gate, but that comes after the synthesized plan is presented, per the stop condition — not a blocker now.

## Caveats
- **Do not invent numbers.** Per repo rules, gambler odds/payouts, vendor prices, and stat magnitudes must be validated against live data tables/source before any tooltip copy is written. Phase 2 (casino/gambler) should not start until those values are confirmed.
- The shared-helper promotion is a refactor with a small blast radius (current callers in `T66StatsPanelSlate` must be repointed) — fine for the implementation phase, but it is a code edit and out of scope for this planning pass.
- Slate `SToolTip` is mouse/hover oriented; the "hover over anything" goal will need a deliberate decision on gamepad/controller focus tooltips later — flag it, don't solve it now.
- This is the independent Validator answer; Codex remains the final router/synthesizer.

</validator_independent_answer>

<codex_draft>
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

</codex_draft>
