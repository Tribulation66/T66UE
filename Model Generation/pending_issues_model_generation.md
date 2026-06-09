# Pending Issues: Model Generation

## Missing FriendSlop Female Hero 1 3D Model [Major]

What's wrong: Hero 1 male has a current FriendSlop raw Pixal3D 3D source, but Hero 1 female does not. The live `CharacterVisuals.csv` rows still point Hero 1 female at older Stacy Animated ToonStyle / Pixal3D ToonStyle assets, and the only located FriendSlop female Hero 1 asset is a 2D reference image.

Why it is out of scope now: This cleanup organized the existing model sources and documentation only. It did not run Pixal3D generation, Unreal import, or Blender look development.

What fixing it would entail: Generate or locate an approved FriendSlop female Hero 1 3D source, place it under the active FriendSlop source-run structure, run the owning Pixal3D import path, and update the live visual rows only after user approval and normal import validation.

