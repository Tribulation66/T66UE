# TemporaryBuffSelection Difference List

Reference: `C:\UE\T66\UI\generation\ReferenceFullScreens_2026-05-02_inventory_locked\TemporaryBuffSelection.png`

Current packaged comparison source used for inventory:
`C:\UE\T66\UI\screens\all_screens\current\2026-05-03\reference12_button_state_sweep\TemporaryBuffSelection.png`

## Visible UI Families

- Fullscreen modal/screen shell with dark wood interior and gold border.
- Four top selected-buff slots with live plus/icon content.
- Chad Coupons live value panel.
- Scrollable buff-card grid with live icons, names, descriptions, owned/slotted counts, and buy/use labels.
- Card action buttons and bottom command buttons.
- Right vertical scrollbar.
- Bottom party tray with four live party/avatar slots.
- Done button, difficulty dropdown, and Enter Tribulation CTA.

## Mismatched Or Missing Families

- `buff-card`: current runtime used the generic tall fullscreen panel as each card, creating oversized gold side columns and arched corners. Reference cards are flatter thin 9-slice frames.
- `top-slot`: current runtime wrapped each empty slot in a row-shell pedestal. Reference slots are standalone framed wells.
- `button-states`: active pill/CTA states were still routed to Shared chrome instead of TemporaryBuffSelection-owned copies.
- `party-slot`: current runtime used generic slot frame styling rather than the screen-owned frame family.
- `coupon-box`, `party-tray`, `dropdown`, and `scrollbar`: still close-enough bootstrap families, but not yet dedicated final art.

## Accepted Runtime Assets

Accepted assets are under:
`C:\UE\T66\SourceAssets\UI\Reference\Screens\TemporaryBuffSelection`

- `Buttons\temporarybuffselection_buttons_pill_normal.png`
- `Buttons\temporarybuffselection_buttons_pill_hover.png`
- `Buttons\temporarybuffselection_buttons_pill_pressed.png`
- `Buttons\temporarybuffselection_buttons_pill_selected.png`
- `Buttons\temporarybuffselection_buttons_cta_normal.png`
- `Buttons\temporarybuffselection_buttons_cta_hover.png`
- `Buttons\temporarybuffselection_buttons_cta_pressed.png`
- `Buttons\temporarybuffselection_buttons_cta_selected.png`
- `Buttons\temporarybuffselection_buttons_cta_disabled.png`
- `Slots\temporarybuffselection_slots_reference_square_slot_frame_normal.png`

No generated images were embedded in chat. This pass reused already-generated/accepted bootstrap component art by duplicating it into the target-owned runtime folder before routing TemporaryBuffSelection to it.

## Runtime Changes

- `Source\T66\UI\Screens\T66TemporaryBuffSelectionScreen.cpp`
  - Routes pill/CTA button states to TemporaryBuffSelection-owned files.
  - Keeps non-square buttons on `T66ScreenSlateHelpers::MakeReferenceSlicedPlateButton` with nearest filtering, zero brush margin, live text, and the shared minimum-width clamp.
  - Switches buff cards from the generic tall fullscreen shell to the quieter target-owned row shell as a 9-slice card contract.
  - Adds a target-owned framed slot surface for top loadout slots and party slots.
  - Removes the empty-slot pedestal shell while preserving live plus/icon state.

## Verification

- `Build.bat T66Editor Win64 Development C:\UE\T66\T66.uproject -WaitMutex`
  - Blocked by unrelated existing compile errors in `Source\T66TD\Private\UI\Screens\T66TDBattleScreen.cpp`.
  - First error: line 724, `SLATE_ARGUMENT(TMap<FName, float>, BattleTuningValues)` expands incorrectly because the comma in the template argument is parsed by the macro.
  - The target T66 module linked during the editor attempt, but the full build failed before a clean packaged proof could be produced.
- `Build.bat T66 Win64 Development C:\UE\T66\T66.uproject -WaitMutex`
  - Blocked by the same unrelated `T66TD` error.

Packaged capture after this pass was not produced because the standalone build cannot complete while the unrelated `T66TD` compile blocker is present. Per the screen-only task boundary, this pass did not edit TD code.

## Remaining Difference List

- `asset`: dedicated final `buff_card_selectable/selected/disabled` art may still be needed if the quieter row-shell card contract is not close enough after packaging is unblocked.
- `asset`: dedicated `coupon_box`, `party_tray`, and `dropdown_normal` target assets are still pending.
- `resize-contract`: the right scrollbar still uses the controls-atlas UV crop and tint; regenerate dedicated vertical track/thumb PNGs if it remains visibly off.
- `layout`: card density, top-slot spacing, and bottom command-strip alignment still need packaged visual confirmation.
- `live-data`: `10 CC` or other account/save values may differ from reference placeholders and should stay live, not baked into art.
