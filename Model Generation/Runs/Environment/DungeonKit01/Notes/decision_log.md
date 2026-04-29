# DungeonKit01 Decision Log

## 2026-04-29

- Created the first modular environment-kit run folder for dungeon walls, floors, and ceilings.
- Pod `69.30.85.145:22083` was bootstrapped to the locked TRELLIS.2 baseline.
- Server health check returned `status=ok` on `http://127.0.0.1:8000/health` from inside the pod.
- First planned module is `DungeonWall_Straight_A`.
- Runtime rule remains unchanged: generated modules are visual overlays first; existing procedural cuboid collision stays authoritative until the kit proves stable.
- Generated six image seeds with Imagegen: four wall modules, one floor tile, and one ceiling tile.
- TRELLIS generated all six raw GLBs from the approved seed images. The first pass incorrectly used Blender Decimate for low-poly review GLBs; those are no longer the accepted Unreal import source.
- The original doorway arch run picked up the green chroma background, so an alpha-background rerun was used for the review scene.
- Alpha reruns for the bones wall and ceiling were attempted but did not produce usable replacements; the original meshes remain in the review set for visual inspection.
- Review scene saved to `Model Generation/Scenes/DungeonKit01_Review.blend`.
- Added a Blender normalization pass that remaps accepted wall/floor meshes into Unreal-sized visual overlay modules.
- Exported five raw-normalized Unreal-ready modules into `UnrealImport`: four walls / doorway modules and one floor tile. Ceiling is intentionally excluded from this pass.
- Built `DungeonKit01_UnrealReady_RoomPreview.blend` from reimported Unreal-ready GLBs to check one-tile room assembly.
- Updated the runtime path so generated wall and floor modules replace old visible procedural wall/floor cube geometry when available; generated meshes are visual-only and hidden simple cube/slab proxies provide gameplay collision.
- Replaced static-mesh collision proxy actors with invisible `UBoxComponent` proxies, corrected wall visual placement for the back-bottom-center wall pivot, and suppressed old visible roof/ceiling cubes while the generated dungeon kit is active.
- Reused the generated floor mesh as the ceiling system: floors spawn an inverted underside visual for the floor below while preserving drop holes, and the top/start floor gets a generated floor-module roof cap with invisible collision.
- Corrected the runtime collision contract so wall collision is spawned per generated visual segment from that segment's transformed mesh bounds instead of one old full-wall box; also authored explicit walkable floor boxes for start/boss room floors so generated floor tiles appear in those rooms.
