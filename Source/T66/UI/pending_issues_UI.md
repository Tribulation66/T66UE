# Pending Issues - UI

## Frontend Screens Lack A Central Controller Focus Contract

- Severity tag: [Major]
- What's wrong: Direct-entry automation can open frontend screens without mouse clicks, but the UI layer does not yet expose a central per-screen controller-focus contract for Steam Deck validation. Individual screens can still vary in their initial focus, directional navigation, accept, and back behavior.
- Why it's out of scope now: This pass builds deterministic screen/run access and avoids mouse automation. It does not retrofit every screen's gamepad focus behavior or add a full controller-navigation smoke matrix.
- What fixing it would entail: Add a focus contract/helper for native and WBP-backed screens, define expected first-focus and navigation rules per screen, then add staged automation that launches each target through direct entry and validates gamepad toggle/accept/back behavior.

## Loot Wheel Boost Rewards Lack A Focused Result Toast

- Severity tag: [Minor]
- What's wrong: `Source/T66/Gameplay/T66LootWheelInteractable.cpp` can lock and commit Gold, Item, or Boost results, but the HUD currently has focused presentation lanes only for gold chest rewards and item pickup cards. Boost results can be committed idempotently after the world spin, but there is no generic stat-boost result card/toast to show after landing.
- Why it's out of scope now: Phase 3 is constrained to applying the animation infrastructure and preserving the existing HUD presentation controller ownership. Adding a new generic reward presentation surface would be a broader UI feature.
- What fixing it would entail: Add a queued generic reward/toast lane to `FT66HUDPresentationController`, support stat-boost title/body/icon data, and route loot wheel boost results through that lane after the landing marker.
