# Original Prompt

Ok lets go ahead and do the consolidated run will not count warning, with also its individual do not ask again check box and this can be consolidated with 5, its basically a general Run will not count and then it says the reason. Either individual you are suspended, or team someone is suspended, or offline, or other issues. Then lets add a settings in gameplay tab, reset all do not show, so basically if they want it to show again, they can press this button. We will have a lot of popups with their own individual do not show so lets make sure our popup, infrastructure is really well build.

# Task Contract

Working task: Build a consolidated "Run will not count" warning infrastructure with per-warning suppression, replace/absorb the existing suspended-party-member warning path into that infrastructure, include individual suspension/team suspension/offline/backend or other unranked reasons, and add a Gameplay Settings reset button for all popup suppressions.
Operator: Codex
Validator: Claude
Scope: Source/Core player settings/popup preference model, Hero Selection run-entry warning behavior and UI, Settings Gameplay reset control, and narrow docs/pending issues if needed.
Stop condition: Scoped changes are made, build/staged verification is attempted or explicitly reported, Claude independent answer and cross-review are incorporated, and final answer reports evidence plus caveats.

# Repo Rules Summary

- Do not use native goal tools.
- Start from live repo state and folder-owned instructions.
- Codex is Operator and Claude is Validator per `.t66/operator-state.json`.
- Claude must be invoked through local Claude Code CLI helpers after verifying `ANTHROPIC_API_KEY` is unset in Process/User/Machine scopes.
- UI work is owned by `UI/UI_AGENTS.md`; backend/ranked/Steam auth wording is owned by `Backend/BACKEND_AGENTS.md`; runtime gameplay changes require compile/build verification and staged standalone validation when they affect the playable standalone.
- Backend-authoritative leaderboard decisions must not be replaced by client-local authority; this popup is user-facing preflight/ineligibility messaging, not final leaderboard acceptance.
