Drop the GPT-generated tower minimap background textures in this folder.

Expected filenames:
- tower_dungeon_background.png
- tower_forest_background.png
- tower_ocean_background.png
- tower_martian_background.png
- tower_hell_background.png

Implementation notes:
- These textures are minimap art only. They sit underneath the native maze wall overlay.
- Unexplored space stays black; discovered space reveals the matching background art.
- Each new tower floor starts unrevealed/black.
- Keep these images square if possible. 1024x1024 is the current expected working size.
- Avoid putting maze walls, icons, or text into these images. The game draws those natively on top.
- Favor broad terrain/material readability over literal ground textures.

Current runtime lookup:
- RuntimeDependencies/T66/UI/Minimap/Backgrounds/tower_dungeon_background.png
- RuntimeDependencies/T66/UI/Minimap/Backgrounds/tower_forest_background.png
- RuntimeDependencies/T66/UI/Minimap/Backgrounds/tower_ocean_background.png
- RuntimeDependencies/T66/UI/Minimap/Backgrounds/tower_martian_background.png
- RuntimeDependencies/T66/UI/Minimap/Backgrounds/tower_hell_background.png
