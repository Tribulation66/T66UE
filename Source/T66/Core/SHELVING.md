# T66 Shelving

Shelving means content is preserved for possible future reactivation, but active player routes are disabled. It is not deletion, deprecation, or archival cleanup.

Runtime ownership is centralized in `FT66ShelvedFeatureGate`:

- `DailyDescent`: frontend/backend daily route disabled.
- `VehicleInteractables`: vehicle prompts/interactables disabled.
- `Pets`: pet selection, pet spawn, pet capture, and pet reward routes disabled.
- `MobLoot`: mob loot world drops disabled.

When adding a shelved feature:

1. Add the feature to `ET66ShelvedFeature`.
2. Set the feature default in `T66ShelvedFeatureGate.cpp`.
3. Gate every player-facing entry point and every runtime spawn/reward path.
4. Keep source/data/assets in place unless the user explicitly asks for archive or deletion.
5. Verify with a focused compile and targeted search for ungated live entry points.
