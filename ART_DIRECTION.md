# T66 Art Direction

## Canonical 3D Direction

FriendSlop is the single canonical art direction for active 3D character, enemy, NPC, interactable, prop, and world visual work.

Current FriendSlop model work uses raw Pixal3D GLB exports, preserves the generated base-color texture identity, and routes through the active FriendSlop import and rigging docs under `Model Generation/Instructions/`.

The material migration target is one shared rubber master material where color is the only per-object variable. The exact lighting model, shading model, and lit-vs-unlit implementation details are still TBD until the rubber-material pass decides and validates them.

## Deprecated 3D Directions

ToonStyle, Animated ToonStyle, QuadRetro, and RetroFX/PS1 documentation are historical art-direction records, not active 3D production routes.

Archived documentation lives here:

- `Archive/ToonStyle/`
- `Archive/RetroFX/`

Do not revive archived art-direction processes, rename runtime asset paths, rename data keys, or rename script/code symbols just to normalize prose terminology. Runtime/data cleanup and material migration require their own approved implementation scope.

## UI Boundary

`UI/FriendslopStyle/` is the active FriendSlop-aligned UI visual track. It shares the FriendSlop brand direction, but it remains a 2D UI pipeline and does not inherit the 3D rubber material contract.

As of 2026-06-09, FriendslopStyle is the active lane for ALL player-facing UI chrome: the
FlatStyle entry points render FriendslopStyle plates globally (`T66.UI.FriendslopGlobal`,
default on), making FlatStyle a legacy-compat adapter layer. The flat-color rendering
remains available behind `-T66FlatLegacy` for debugging only.

RetroFX settings documentation for live UI screens is historical after the 2026-06-07 product decision to archive the player-facing Retro FX Settings tab. The runtime RetroFX subsystem and compatibility helpers may remain for legacy/preview flows, but do not restore the Settings tab without a new product decision.
