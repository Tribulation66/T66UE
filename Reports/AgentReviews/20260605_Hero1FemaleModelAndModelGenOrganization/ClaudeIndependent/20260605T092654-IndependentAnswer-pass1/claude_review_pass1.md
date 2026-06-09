Result: OK

## Independent Answer

**1. Which model Hero 1 female currently uses.** Hero 1 female is "Stacy" (`Content/Characters/Heroes/Hero_1/Stacy`). Her shipped skins are the existing Pixal3D/AccuRig-lineage set: default `Pixal3DToonStyle/SM_Hero_1_Stacy` + outline, the rigged `AnimatedToonStyle/SK_Hero_1_Stacy`, plus DemoSkin, Beachgoer, and QuadRetro variants. **No FriendSlop-generated Hero 1 female model exists.** The new FriendSlop runs (`FriendSlopProbe_Hero1Male_20260604_1415` and `FriendSlopEasyBatch_20260604_1532`) cover only Hero 1 *male* among heroes — the batch is boosts, gates, dungeon enemies, NPCs, companions, pets, and props; Stacy/Hero_1 female is not in either run. So unlike male (confirmed FriendSlop), the female still rides the older Pixal3D source lineage.

**2. Pixel3D/source-image change vs. Blender material work.** For a Fall Guys rubber/jelly look, Blender material/look-dev is the higher-leverage lever. The rubber appearance is a shader-response property — roughness, specular/sheen, subsurface, clearcoat, normal softening — none of which the source image controls. Pixel3D/source-image regeneration mainly changes albedo and silhouette, not material response, and risks invalidating the already-correct male and the existing female mesh. Recommend: keep the existing meshes, do the rubber look in Blender material/look-dev, treat source-image regeneration as a last resort only if silhouette/topology blocks the material.

**3. Organization cleanup (Codex/Operator to execute — I cannot edit).** Path-safe plan:
- Active source set = the two FriendSlop runs under `Model Generation/Runs/Pixal3D/`. Surface them as the current source location and reference them in the README/Instructions.
- Archive legacy non-current generations, notably `Runs/Pixal3D/HeroDemoLineup_20260522_AccuRig` (and any other pre-FriendSlop hero batches), into an `Archive/` subtree with a short provenance note. Move only, no deletion.
- README/`Instructions` edit: state that current model work uses FriendSlop Pixal3D models; AccuRig batches are archived reference only. Note that `RR_accurig_textured_export` filenames inside the FriendSlop runs are just the export-stage naming, not the legacy AccuRig source set — call this out so future agents don't confuse them.

## Evidence Checked
- `.t66/operator-state.json` (Codex Operator, Claude Validator).
- `Model Generation/README.md`, `MODEL_GENERATION_AGENTS.md`, Pixal3D dir listing.
- `Content/Characters/Heroes/Hero_1/Stacy` skin tree (Pixal3D/Animated/DemoSkin/Beachgoer/QuadRetro).
- `Runs/Pixal3D/FriendSlopProbe_Hero1Male_*` and `FriendSlopEasyBatch_*` member lists — no Stacy/female hero present.

## Questions Or Blockers
None requiring the user. One thing Codex must state plainly rather than gloss: there is **no newly generated FriendSlop female hero model to place**, so the "put newly generated FriendSlop models in the clear location" step applies to male/the batch assets, not female. If the user actually wants a FriendSlop *female* regeneration, that conflicts with "use the existing model for next Blender work" — but the user already chose the existing model, so proceed.

## Caveats
- "Current female model" is inferred from on-disk skin folders; the authoritative wiring lives in the Stacy character Blueprint/skin data asset, which I did not open. Folder layout strongly implies `Pixal3DToonStyle` default + `AnimatedToonStyle` rig, but confirm in the BP if it matters.
- I did not enumerate every legacy run; Codex should sweep `Runs/Pixal3D/` for any other pre-FriendSlop hero batches before declaring the archive complete.
- Archiving is a move, not a delete — verify no script/manifest references the old `HeroDemoLineup` paths before relocating, or those references will break.
