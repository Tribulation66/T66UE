# Pending Issues - UI

## Frontend Screens Lack A Central Controller Focus Contract

- Severity tag: [Major]
- What's wrong: Direct-entry automation can open frontend screens without mouse clicks, but the UI layer does not yet expose a central per-screen controller-focus contract for Steam Deck validation. Individual screens can still vary in their initial focus, directional navigation, accept, and back behavior.
- Why it's out of scope now: This pass builds deterministic screen/run access and avoids mouse automation. It does not retrofit every screen's gamepad focus behavior or add a full controller-navigation smoke matrix.
- What fixing it would entail: Add a focus contract/helper for native and WBP-backed screens, define expected first-focus and navigation rules per screen, then add staged automation that launches each target through direct entry and validates gamepad toggle/accept/back behavior.
