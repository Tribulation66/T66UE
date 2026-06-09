Result: OK

## Independent Answer

Build this as a **keyed popup-suppression model**, not another one-off boolean. The current code has two ad-hoc flags on the SaveGame (`bShowPartySuspendedLeaderboardPopup`, `bShowRunSummaryChadCouponsPopup`) gated through individual getters/setters. Adding more popups this way doesn't scale and makes "reset all" awkward. The user explicitly asked for infrastructure that's "really well built" because "we will have a lot of popups," so:

1. **Suppression store**: Add a keyed collection to `T66PlayerSettingsSaveGame.h` — e.g. `TSet<FName> SuppressedPopups` (or `TMap<FName,bool>`). Each popup gets a stable `FName` key (e.g. `RunWillNotCount`, `RunSummaryChadCoupons`). Subsystem exposes generic `IsPopupSuppressed(FName)`, `SetPopupSuppressed(FName,bool)`, and `ResetAllPopupSuppressions()`. The reset button just clears the set and saves.

2. **Schema migration**: Bump `T66PartySuspendedPopupSchemaVersion` (currently 25, see `T66PlayerSettingsSubsystem.cpp:30`) and migrate the existing two booleans into the keyed set so users who already checked "don't show again" keep that state. Keep the old fields readable for the migration step.

3. **Consolidated "Run will not count" popup**: One popup type that takes a *reason* (individual-suspended / team-member-suspended / offline / backend-or-other-unranked). Title is constant; body text is reason-driven. Replace the `ShowPartyLeaderboardRestrictionWarning` path (`T66HeroSelectionScreen.cpp:782`) and `HandlePartyLeaderboardRestrictionOkayClicked` (`:322`) so the team-suspended case becomes one reason inside the consolidated popup. The "consolidate with 5" appears to mean folding the previously-discussed offline/unranked notice into this same popup — Codex should treat it as the offline reason branch unless the user's list says otherwise.

4. **Preserve backend authority**: This popup is preflight/ineligibility messaging only. Final eligibility stays on `GI->bRunIneligibleForLeaderboard` / backend; do not let the popup or its suppression decide leaderboard acceptance. Suppressing the popup must **not** suppress the actual ineligibility flag — a suppressed warning still produces an unranked run.

5. **Gameplay reset control**: Add a "Reset all 'do not show again' popups" button in `T66SettingsScreen_Gameplay.cpp` using the existing `MakeFlatButton` helper (seen at `:281`), wired to `ResetAllPopupSuppressions()`.

6. **Verification**: This is runtime gameplay UI, so repo rules require compile/build plus staged standalone validation (enter Hero Selection with a suspended party member, confirm consolidated popup + don't-show-again persists, confirm reset re-enables).

## Evidence Checked
- `.t66/operator-state.json` — Codex Operator / Claude Validator confirmed.
- `T66PlayerSettingsSaveGame.h:126,130` — existing per-popup booleans.
- `T66PlayerSettingsSubsystem.cpp:30,314-317,630-642` — schema versioning + party-suspended getter/setter.
- `T66HeroSelectionScreen.cpp:322-340,782-794,840-865` — existing suspended-party warning, don't-show-again handling, party broadcast, `bRunIneligibleForLeaderboard`/`IsOfflineRun` wiring.
- `T66SettingsScreen_Gameplay.cpp:236-281,479-501` — toggle-row and flat-button helpers available for the reset control.

## Questions Or Blockers
None that require the user. The "consolidate with 5" reference points to an item in the user's own mental list; Codex can resolve it as the offline/unranked reason branch and note the interpretation in the final answer.

## Caveats
- **Ownership split**: the popup UI is owned by `UI/UI_AGENTS.md`, but the user-facing *wording* for suspension/offline/ranked reasons is backend/ranked territory (`Backend/BACKEND_AGENTS.md`). Codex should source the reason strings from backend-owned conventions, not invent ranked wording in the UI layer.
- Keep the migration non-destructive — a botched schema bump could silently re-enable popups users already dismissed.
- Don't conflate suppression with eligibility (item 4) — that's the one way this change could accidentally violate the backend-authority rule.
- Build/standalone verification is mandatory here and must be reported in the final answer, not assumed.
