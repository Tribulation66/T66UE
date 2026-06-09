Result: OK

## Independent Answer

The change is well-scoped and Codex can implement it internally. Here is the concrete shape and the one real gotcha.

**Where to edit:** `Source\T66\UI\Screens\T66PauseMenuScreen.cpp` / `.h`.

**What to add:**
1. Localized label — `GetText_Quit()` already exists (`T66LocalizationSubsystem.h:134`, `.cpp:853`) and is used by the frontend top bar (`T66FrontendTopBarWidget.cpp:977`). Reuse it; no new loc key needed.
2. A new button slot in `BuildSlateUI()` alongside the existing six, plus a `HandleQuitClicked()` → `OnQuitClicked()` pair mirroring the existing handler pattern (`.cpp:228-233`, `.h:45-51`).
3. Quit behavior must route through the existing shutdown owner, exactly as `T66QuitConfirmationModal::OnQuitClicked()` does (`T66QuitConfirmationModal.cpp:156-168`): `UT66ShutdownSubsystem::RequestQuitGame(ET66ShutdownReason::UserQuit, 0)` with the `UKismetSystemLibrary::QuitGame` fallback. This satisfies the "use existing shutdown owner" rule and the "no native goal tools" rule. The cleanest path is to reuse the existing confirmation flow: `ShowModal(ET66ScreenType::QuitConfirmation)` (the pause menu already uses `ShowModal` for Settings/Achievements at `.cpp:297,303`), which then drives the shutdown subsystem — avoids duplicating shutdown logic and gives the user a confirm step.

**The gotcha (layout — this is the substantive validator finding):** The six current buttons already fill the modal panel. Panel is `AddSlot(657, 121, 605, 838)` → bottom at Y=959. Buttons sit at Y = 295/407/519/631/744/856 (112px pitch, 95px tall); the last ends at Y=951, just inside the panel. A naive 7th button at Y=968 would end at Y=1063 — overflowing both the panel (959) and effectively the 1080 canvas. So this is **not** a one-line append: Codex must rework the vertical layout (tighten pitch / shrink buttons / grow the panel) and keep it consistent with `UI/Geometry/pause_menu_structural_inventory.md` and the sizing rules in `UI/Instructions/UI_LAYOUT_AND_SIZING_INSTRUCTIONS.md`. Update that geometry inventory + `UI/Checklists/pause_menu_checklist.md` to match.

**Verification:** per the stop condition, run the focused pause-menu smoke/visual check (there is a `pause_menu_checklist.md` and frontend tag-click smoke scripts under `Scripts\`) or clearly caveat if not run, and report exact proof (the new button tag, e.g. `PauseMenu.QuitButton`, and the shutdown call site).

## Evidence Checked
- `T66PauseMenuScreen.cpp` / `.h` — full button list, slot offsets, handler pattern, panel geometry.
- `T66QuitConfirmationModal.cpp` — canonical quit path via `UT66ShutdownSubsystem::RequestQuitGame` + fallback.
- `T66FrontendTopBarWidget.cpp:977` — existing `GetText_Quit()` usage.
- `T66LocalizationSubsystem.h/.cpp` — `GetText_Quit()` exists.
- Existing `Save and Quit` (`OnSaveAndQuitClicked`, `.cpp:254`) returns to frontend via SessionSubsystem — to be left intact.

## Questions Or Blockers
None that require the user. Two defaults Codex should pick and state explicitly in its proof:
- **Quit semantics:** "Quit" here should mean quit-to-desktop via the shutdown owner (matching the frontend Quit and the "use existing shutdown owner" rule), distinct from "Save and Quit" which returns to frontend. If the user actually meant "quit to main menu without saving," that differs — but quit-to-desktop is the defensible default.
- **Confirmation:** reuse the existing `QuitConfirmation` modal rather than quitting instantly (avoids accidental loss of unsaved run progress).

## Caveats
- This touches sizing/layout, which is governed by `UI/UI_AGENTS.md` and the layout/sizing instructions — the panel/button rework must follow those, and the geometry inventory + checklist must be updated, or it'll fail UI-owner review.
- Quitting from a paused state: ensure the shutdown path doesn't depend on unpausing first (Save and Quit explicitly calls `SetPause(false)` at `.cpp:260`); Codex should confirm the shutdown subsystem handles a paused world or unpause before requesting quit.
- I did not build or run anything (read-only validator); the overflow conclusion is from the static offsets above and should be confirmed visually.
