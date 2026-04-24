# Temporary Buff Selection Layout List

Canvas: 1920x1080.

Source evidence:
- `Source/T66/UI/Screens/T66TemporaryBuffSelectionScreen.cpp:419` sets a near-full-safe-frame modal, `0.965x` width and `0.94x` height, with minimum `960x720`.
- `Source/T66/UI/Screens/T66TemporaryBuffSelectionScreen.cpp:422` fixes the buff grid at 6 columns with 236 px card height.
- `Source/T66/UI/Screens/T66TemporaryBuffSelectionScreen.cpp:433` through `443` define the title, helper copy, action labels, and Chad Coupon value.
- `Source/T66/UI/Screens/T66TemporaryBuffSelectionScreen.cpp:476` through `486` builds 4 loadout slots from selected single-use buff state.
- `Source/T66/UI/Screens/T66TemporaryBuffSelectionScreen.cpp:715` through `829` builds the scrollable buff-card grid from `GetAllSingleUseBuffTypes()`.
- `Source/T66/UI/Screens/T66TemporaryBuffSelectionScreen.cpp:852` through `965` defines the full modal region order.
- `Source/T66/Core/T66BuffSubsystem.h:36` and `37` set single-use buff cost to 1 CC and max selected buffs to 4.

Required image-generation inputs:
- Style anchor: `C:\UE\T66\UI\screens\main_menu\reference\canonical_reference_1920x1080.png`
- Current target screenshot: `C:\UE\T66\UI\screens\temporary_buff_selection\current\2026-04-24\current_1920x1080.png`.
- Layout list: `C:\UE\T66\UI\screens\temporary_buff_selection\layout\layout_list.md`
- Generated screen-specific reference: TODO/missing. Required path is `C:\UE\T66\UI\screens\temporary_buff_selection\reference\canonical_reference_1920x1080.png`.

Reference gate status: blocked. The canonical main-menu anchor and current target screenshot exist, but the generated screen-specific reference is missing. Do not proceed to sprite generation, Slate/C++ placement, or packaged comparison for this screen until the generated target reference exists.

## Regions

- Full-screen scrim: runtime modal backdrop dimming the previous screen; generated reference may show the dimmed context, but no production art should bake the underlying screen.
- Main modal shell: centered near-full-screen content frame, fixed around the safe frame, with dark fantasy panel treatment.
- Header title block: left side title `SELECT TEMP BUFFS` plus helper copy `Click a top slot, then pick a buff below.`; runtime/localizable text.
- Chad Coupons well: right header mini-panel with `CHAD COUPONS` label and live `{balance} CC` value.
- Loadout slot row: 4 top slots, each with icon or `+`, focused slot highlight, owned/unowned visual tint, and optional slot-level action.
- Buff grid scroll region: 6-column grid of live buff cards inside a scroll box; current source expects 28 live cards from the filtered single-use buff list.
- Buff card: icon well, buff name, description, owned count, slotted count, and action button.
- Empty grid state: fallback centered message if the filtered buff list is empty.
- Footer party panel: 4 party avatar slots with fallback initials or slot numbers; host/local slot has a different accent.
- Footer Done control: compact neutral `DONE` button closes the modal.
- Footer run controls: difficulty dropdown plus green `ENTER TRIBULATION` call-to-action.
- Difficulty menu: dropdown list for Easy, Medium, Hard, Very Hard, and Impossible.

## Panels And Shells

- Modal content shell: generated frame/backplate, no baked text.
- Row/card shell: reusable generated small panel for loadout slots, party well, run controls, and buff cards.
- Buff card shell: generated card frame with open icon, text, count, and button wells.
- Loadout slot shell: generated compact frame with selected/focused highlight support; icon remains runtime-owned.
- Coupon well shell: generated compact counter frame; number remains runtime-owned.
- Footer party shell: generated strip with 4 avatar sockets.
- Button plate families: compact neutral, toggle-on/selected, toggle-inactive/disabled, and green CTA; labels remain runtime-owned.

## Controls And Required States

- Loadout slot button: real visible button; states `normal`, `focused`, `filled`, `empty`, `owned`, `unowned`, `hover`, `pressed`.
- Loadout slot action button: `CLEAR` when owned/filled, `BUY` when filled but unowned; states `normal`, `hover`, `pressed`, `disabled`.
- Buff card action button: `USE` when an owned copy is available, `BUY {cost} CC` when a copy must be bought; states `normal`, `hover`, `pressed`, `disabled`, `selected/focused-match`.
- Buff card shell: states `normal` and `focused-slot-matches`.
- Difficulty dropdown: closed, open menu, hovered option, selected option.
- Enter Tribulation button: green CTA; states `normal`, `hover`, `pressed`, `disabled`.
- Done button: compact neutral; states `normal`, `hover`, `pressed`, `disabled`.
- Scroll region: scrollbar states `normal`, `hover`, `dragged`, `disabled` if visible.

## Live Runtime Content

- Live/localizable text: title, hint copy, `CHAD COUPONS`, loadout `+`, `USE`, `BUY`, `CLEAR`, `DONE`, `ENTER TRIBULATION`, difficulty names, buff names, buff descriptions, party header.
- Live values: Chad Coupon balance, single-use buff cost, owned count per buff, assigned/slotted count per buff, selected difficulty, focused slot index.
- Live images/media: secondary buff icons from `RuntimeDependencies/T66/UI/PowerUp/SecondaryBuffs/*.png`, Steam party avatars, local avatar fallback.
- Live state: selected loadout slots, whether each selected slot is owned, whether a card can use an owned copy, whether the player can afford a purchase, party membership, avatar availability.
- Strict diff strategy: use a staged save/profile with deterministic selected slots, coupon balance, party state, and scroll position; mask avatar wells and any dynamic text counts if fixture data is unavailable.

## Variants

- Empty loadout slots with no selected buffs.
- Filled owned loadout slot with visible icon and `CLEAR`.
- Filled unowned loadout slot with dimmed icon and `BUY`.
- Focused empty slot.
- Focused slot matching a buff card.
- Owned buff card with `USE`.
- Unowned/insufficient-coupon buff card with disabled purchase.
- Unowned/affordable buff card with enabled `BUY {cost} CC`.
- Difficulty dropdown open.
- Empty buff list fallback.
- Party empty, local-only, and 4-member party avatar states.

## Reference Generation Constraints

- Preserve the near-full-screen modal with header, 4 loadout slots, scrollable 6-column card grid, and footer controls.
- Match the canonical main-menu material language: dark readable panel faces, chunky fantasy frames, warm metal/gem accents, green CTA, and blue/purple/neutral secondary controls.
- Do not bake localizable labels, buff names/descriptions, counts, coupon values, difficulty names, button labels, or party names into runtime-intended sprites.
- Leave icon wells and avatar wells open for runtime images.
- The generated full reference may include representative readable text for planning, but later sliceable runtime art must be text-free except approved display art.
