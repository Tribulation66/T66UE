I have conclusive CSV evidence. Hero 1 female maps to "Stacy" and does not use FriendSlop.

Result: OK

## Independent Answer

**Hero 1 female does NOT currently use a FriendSlop model.**

The runtime visual mapping lives in `Content/Data/CharacterVisuals.csv`. In this project Hero 1 male = "Chad" and Hero 1 female = "Stacy". The relevant rows:

- `Hero_1_Chad` (male) → StaticMesh `/Game/Characters/Heroes/Hero_1/Chad/**FriendSlopRaw**/SM_Hero_1_Chad_Male` — this is the confirmed FriendSlop model, matching the user's statement that male is correct.
- `Hero_1_Stacy` (female) → SkeletalMesh `/Game/Characters/Heroes/Hero_1/Stacy/**AnimatedToonStyle**/SK_Hero_1_Stacy` + StaticMesh `/Game/Characters/Heroes/Hero_1/Stacy/**Pixal3DToonStyle**/SM_Hero_1_Stacy` (+ Outline + walk/idle/jump/roll anims).
- `Hero_1_Stacy_DemoSkin` similarly uses AnimatedToonStyle / Beachgoer Pixal3DToonStyle, no FriendSlop path.

So female is still on the older AnimatedToonStyle (skeletal) + Pixal3DToonStyle (static) pipeline, not the FriendSlopRaw path the male uses. Female is not a FriendSlop model.

**Pixal3D/source-image changes vs Blender material work for the rubber/vinyl (Fall Guys-like) target:**

For a rubber/vinyl look, Blender material/look-dev has the higher expected value. The rubber appearance is dominated by surface shading response — soft specular rolloff, subtle subsurface/sheen, smooth normals, low roughness variation. Those are material/shader properties, not source-pixel or generator-topology properties. Pixal3D/source-image changes mainly affect base color, silhouette, and toon-texture detail; they cannot create the lighting-dependent rubber response. Reserve Pixal3D/source-image work for cases where the base albedo or shape is wrong, which is a separate problem from the rubber finish. Recommendation: drive the rubber target through Blender material/look-dev on the existing model (consistent with the user's confirmed decision to keep the existing model), and only touch Pixal3D/source if albedo/silhouette issues surface during look-dev.

## Evidence Checked

- `Content/Data/CharacterVisuals.csv` — Hero_1_Chad, Hero_1_Chad_DemoSkin, Hero_1_Stacy, Hero_1_Stacy_DemoSkin rows (live read).
- `FriendSlop/ImageGen/Heroes/Demo/Hero_1_Chad_Female/...ReferenceRepro` manifest exists (a female reference-repro image-gen artifact), but it has not been wired into the CSV runtime path.
- Naming convention confirmed in-repo: Chad = male, Stacy = female across all Hero rows.

## Questions Or Blockers

None requiring the user. The model-path question is answered by the CSV with no ambiguity, and the recommendation is within model judgment.

## Caveats

- I inspected the CSV path references, not the rendered assets; whether the engine renders the skeletal (AnimatedToonStyle) or static (Pixal3DToonStyle) variant at runtime depends on selection logic, but neither is a FriendSlop model, so the answer holds either way.
- The existence of a `Hero_1_Chad_Female` FriendSlop ImageGen manifest suggests a female FriendSlop source may be in progress; if the user's intent is to also migrate female to FriendSlop, that is a separate (currently unwired) effort, not the current runtime state.
- Rubber-look recommendation is a qualitative look-dev judgment; final tuning should be validated visually in Blender against the Fall Guys reference before committing.
