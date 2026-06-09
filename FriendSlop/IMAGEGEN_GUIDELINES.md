# FriendSlop Image Generation Guidelines

This is the active FriendSlop image-generation guide. All previous visual
branches are retired and have been removed from the working reference set.

## Current Locked References

Use only these two images as the visual baseline:

- Male: `C:\UE\T66\FriendSlop\ImageGen\Heroes\Demo\Hero_1_Chad\FriendSlop_Hero1_Chad_Male_ReferenceRepro_v02.png`
- Female: `C:\UE\T66\FriendSlop\ImageGen\Heroes\Demo\Hero_1_Chad_Female\FriendSlop_Hero1_Chad_Female_ReferenceRepro_v01.png`

These are the rose-pink skin-tone founding-era Hero 1 references.

No other image in the repo should be treated as a FriendSlop style reference.

## Style Target

FriendSlop now means reproducing the new rose-pink Hero 1 direction:

- black background
- full-body front-facing model-sheet render
- smooth stylized 3D form language
- rounded emoji-like face
- half-lidded simple eyes
- small curved mouth
- white founding-era hair
- navy and cream founding-era outfit
- warm rose-pink nude skin tone
- broad simple material regions
- hands shaped like oven mitts: one thumb plus one merged mitten mass
- clean, readable silhouettes for later 3D conversion

The style should feel like a direct family of the two locked references, not a
new reinterpretation.

## Shared Pose Rules

For humanoids:

- full-body orthographic front view
- one subject only
- head, torso, hips, knees, and feet face forward
- neutral A-pose with arms relaxed away from the torso
- hands visible and empty
- feet point forward
- no cropped body parts

For non-humanoids:

- one subject only
- front-facing model-sheet presentation
- full body visible with generous padding
- preserve the live game silhouette and role first
- translate the subject into the same material, face, and simplicity language as
  the locked references

## Creature Translation Rules

Enemies, pets, and bosses should:

- keep the black-background render language from the locked references
- use the same warm, smooth, simplified 3D material logic
- give creatures simple emoji-like facial reads where appropriate
- use broad readable color regions
- avoid tiny texture noise and excessive surface detail
- preserve the core game identity from the data row

Example: Slime should be a clean stylized green blob in the locked reference
render language.

## Mob Loot Rule

Mob Loot should be visually plain and readable:

- small pile of bones and skulls
- cream bone-white rounded forms
- no treasure, coins, glow, rarity effects, or ornate presentation
- same black-background model-sheet format as other FriendSlop source images

## Hand Rule

For any humanoid or humanoid-adjacent subject:

- hands are mitten hands
- each hand has one rounded thumb and one merged rounded mass for the rest of
  the fingers
- no individual fingers
- no finger grooves
- no claws unless the specific creature identity requires claws

## Output Convention

For each accepted source candidate:

- save the portrait source candidate under `FriendSlop\ImageGen\<Category>\<Target>\`
- save the exact prompt beside it
- save a short manifest beside it
- do not create comparison sheets unless explicitly requested

## Current Priority

The next generation pass should use the locked male/female references above as
the only style source.
