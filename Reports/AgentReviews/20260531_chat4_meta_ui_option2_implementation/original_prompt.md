CHAT 4 - META/UI implementation, option 2.

Build now to the Foundation seam, with Mob Loot targeting and collection disabled until the Foundation API exists.

Required scope:
- Hero selection: remove the right-column VideoPanel only, not the central HeroPreviewController path. Add kit preview panel above stats, clickable Weapon and Ultimate sections, real selected-clip routing, video catalog kit-preview lookup with frontend video player fallback, Choose Pet button, and PetSelection screen modeled on companion selection.
- Kit display data must come through a disposable read-only adapter over existing FHeroData, Weapons.csv, and weapon manager until Combat resolver lands. Do not duplicate Combat kit definitions in UI.
- Pets: add data keyed by boss IDs, append-only save fields and migration, PetCaptureInteractable at boss death location, one-active-pet follow behavior, movement-speed-only union/bond, first capture auto-equips only if no active pet.
- Capture rules: only after RemainingBossCount == 0, use existing bCompletedSelectedDifficulty branch to skip final difficulty-ending bosses, non-final stage bosses only, repeat kills of captured bosses spawn nothing.
- Shared boss defeat orchestration: pet capture for non-final bosses, then Combat idol altar, then gate/summary.
- Defer Mob Loot claim/walk/collect until Foundation publishes SpawnMobLoot, CollectMobLootAt, and reservation API. Do not scan loot bags, use loot-bag registry, or reimplement collection.
- No git operations.

