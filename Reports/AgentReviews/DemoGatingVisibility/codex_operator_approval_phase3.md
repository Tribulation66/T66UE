Codex Approval: APPROVE

## Approved Task

Phase 3 of Demo Gating Visibility: hide demo-gated entries from visible non-Mini UI instead of showing `COMING SOON`, and update the demo docs/rules to reflect the hidden-entry model.

## Approved Scope

Approved edits are limited to:

- `Source/T66/UI/Screens/HeroSelection/T66HeroSelectionScreen_Build.cpp`
- `Source/T66/UI/Screens/HeroSelection/T66HeroSelectionScreen_Party.cpp`
- `Source/T66/UI/Screens/T66HeroGridScreen.cpp`
- `Source/T66/UI/Screens/T66CompanionSelectionScreen.cpp`
- `Source/T66/UI/Screens/T66CompanionGridScreen.cpp`
- `Source/T66/UI/Screens/T66MainMenuScreen.cpp`
- `Source/T66/UI/Components/T66FlatLeaderboardPanel.cpp`
- `Source/T66/UI/Gambler/T66CasinoGamblerTabWidget_Build.cpp` and `Source/T66/UI/T66CasinoGamblerTabWidget.cpp` only if needed to hide demo-gated casino-game entries.
- `Demo/DEMO_RELEASE_INSTRUCTIONS.md`
- `Demo/DEMO_GATED_INVISIBLE_CONTENT.md`

Approved behavior:

- Use existing release-variant playable/allowed lists to omit non-demo heroes, companions, difficulties, Lab, Daily Descent, and other non-deprecated demo-gated UI entries from visible lists/buttons.
- Keep backend/navigation guards in place.
- Leave the shared overlay helper intact for code that still intentionally uses it.
- Update docs so future agents know demo-gated content should be hidden, not shown as `COMING SOON`.

## Approved Tool Surface

Claude FullOperator may read/edit approved files, run focused searches over `Source/T66/UI`, `Source/T66/Core`, `Config`, and `Demo`, and run a focused T66 C++ compile. UI capture may be produced if practical, but the final combined staged/capture proof remains Phase 4.

## Required Process Rules

- Follow `AGENTS.md`, `OPERATOR_VALIDATOR_PROTOCOL.md`, `Demo/DEMO_AGENTS.md`, `UI/UI_AGENTS.md`, and `Reports/AGENTS.md`.
- Do not use native goal tools.
- Preserve user changes outside approved files.
- Do not edit Mini/minigame/deprecated runtime code. Deprecated arcade/minigame overlay code can remain because it is tracked separately in `Demo/DEPRECATED_CONTENT.md`.
- Keep docs and code consistent with `Demo/DEMO_GATED_INVISIBLE_CONTENT.md`.

## Explicitly Excluded Actions

- No Mini/minigame runtime code, assets, captures, or implementation changes.
- No deprecated-feature code changes.
- No arcade deprecated runtime changes.
- No Git commit, push, tag, reset, checkout, clean, or broad LFS scan.
- No staged standalone refresh in this phase; Phase 4 owns staging and shortcut verification.

## Verification Required After Operator Run

- Report exact files changed and key behavior changes.
- Run focused C++ compile and report pass/fail markers.
- Provide code-level proof that non-Easy difficulties, non-allowed heroes, non-allowed companions, Daily Descent, and Lab no longer render as visible `COMING SOON` entries in the targeted non-Mini UI.
- Report any remaining `WrapWithComingSoonOverlay` or unavailable overlay usages and classify them as either available-now no-op, deprecated/Mini excluded, or still needing follow-up.

## Approval Rationale

This phase implements the user's requested visible UI concept after the available-content and inventory phases. It remains bounded to non-Mini visible UI and docs, with staged/capture proof reserved for the final phase.
