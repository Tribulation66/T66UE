# Decision Block — Professional Minimap Redesign

Date: 2026-06-09

## Decision (made by user)

The minimap should be **circular** with a **Fall Guys "inflatable bouncy house" colorful theme**
(soft, rounded, glossy/inflatable, candy-bright palette). This aligns with the FriendSlop brand
direction and the FriendslopStyle UI track.

## Implications locked

- Shape: round minimap (replaces the current square FlatStyle panel border).
- Aesthetic: inflatable/bouncy — thick rounded ring frame with soft gloss highlight + drop shadow,
  saturated playful colors, rounded markers. Cohesive with the bouncy-rubber FriendSlop world.
- Owning process: UI (`UI/UI_AGENTS.md`) → FriendslopStyle look, FlatStyle/Slate-native chrome rules
  still apply unless raster chrome is routed through the FriendslopStyle process.
- Verification: focused C++ build + Unreal-owned capture of the HUD minimap widget.

## Out of scope (unless user later asks)

- Full-map (M-screen) reskin beyond what cohesion requires.
- Gameplay marker semantics / data (enemy/POI/trap logic stays as-is).
- 3D world / rubber material work.
