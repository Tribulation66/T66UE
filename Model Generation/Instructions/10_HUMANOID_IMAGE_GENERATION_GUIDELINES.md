# Humanoid Image Generation Guidelines

Use this file for T66 humanoid source images: heroes, companions, humanoid
bosses, and other manually rigged characters. These rules sit above individual
prompt files and below the shared source-image gate in
`02_SOURCE_IMAGE_RULES_INSTRUCTIONS.md`.

## Scope

Use this guide when the output will become a full-body humanoid source image
for Pixal3D, TRELLIS, AccuRig, or later ToonStyle import.

Do not use this guide for non-humanoid mobs, props, interactables, environment
modules, UI portraits, marketing art, or contact sheets.

## Read Order

1. Read `02_SOURCE_IMAGE_RULES_INSTRUCTIONS.md`.
2. Read this humanoid guideline.
3. Read the target-specific prompt or identity note.
4. Generate one target per prompt. Do not paste unrelated hero identities into
   the live prompt.
5. Review the source image before sending it downstream.

## Locked Humanoid Pose

- Orthographic full-body front-view model-sheet stance.
- One subject only, with clear subject/background separation.
- Head, torso, hips, knees, and feet face squarely forward.
- Use rigging-friendly A-pose 1.5: arms angled slightly downward from the
  shoulders, between A-pose 1 and A-pose 2.
- Arms stay away from the torso with a clear white gap under each armpit.
- Elbows are mostly straight and relaxed.
- Hands are visible, empty, and do not touch the torso, hips, thighs, coat,
  armor, or boots.
- Legs are straight under the hips in a rigging-safe stance, not a wide power
  stance.
- Feet are narrow hip-width, directly under the hip sockets, not shoulder-width.
- Thighs, knees, calves, and boots do not touch. Keep a clear vertical white gap
  between the legs from crotch to boots.
- Knees point straight forward.
- Boots and feet face forward or slightly inward toward the centerline.
- Boot toe caps face the viewer directly. Do not show the side profile of either
  boot.
- Inner and outer boot edges are vertical and parallel or very slightly inward.
- Boots stay inside shoulder width and do not spread outward.

Reject outward-facing feet, V stance, cowboy stance, action stance, battle
stance, ballet stance, crossed legs, knees touching, thighs touching, wide
planted stance, three-quarter turn, or cropped limbs.

## Shared Style Lock

- Clean ToonStyle / Guilty Gear-inspired anime fighting-game source art.
- Mature fighting-game faces with angular planes, sharp cheek structure, strong
  brow ridge, narrow intense eyes, defined nose bridge, firm mouth, and hard
  cel-shaded facial planes.
- Avoid soft cartoon faces, simplified children's anime, chibi features, round
  cheeks, tiny noses, blank doll faces, or generic cute anime styling.
- Use crisp ink contours, hard color boundaries, and broad cel-art planes.
- Avoid realistic lighting, cast shadows, contact shadows, ambient-occlusion
  shadow masses, airbrushed mobile-game shading, painterly rendering, grain,
  dithering, fabric noise, scratches, weathering, and texture noise.

## Color And Detail Discipline

Humanoids may use more colors than the general two-color creature/prop rule
when the colors are assigned to large dedicated body or clothing sections.

For normal humanoid generations, target these readable sections:

- one skin color
- one hair color
- one shoe or boot color
- two to four major outfit colors, each owned by a specific garment section

Rules:

- Use large dedicated material-color regions only.
- Each color belongs to one major section. Do not mix colors randomly across a
  garment.
- Clothing colors must not blend with skin color.
- Avoid white, cream, tan, or skin-like clothing next to exposed skin unless the
  target identity explicitly requires it and the boundary remains clear.
- No thin trim, filigree, scrollwork, tiny emblems, repeated studs, jewels,
  ornamental knots, small floating details, busy lace, tiny jewelry, pouches,
  straps, fabric texture, or button rows.
- Buttons are allowed only when they are large, few, aligned, and do not break
  the main section read.
- Allowed detail is limited to major garment boundaries and broad
  anatomy-following contour planes.

## Clothing Shape Rules

- Pants, leggings, fitted trousers, or another clear lower-body garment are
  required unless Pablo explicitly asks for a skirt-only character.
- Any coat, tunic, skirt panel, armor flap, robe, sash, apron, or garment that
  drops below the waist must stop at mid-thigh or higher.
- Nothing below the waist may reach the knees, kneecaps, lower thighs, calves,
  boots, or boot tops.
- Clothing and armor should behave like fitted heroic fabric or fitted armor
  panels, not loose cloth that hides the body silhouette.
- Do not use weapons, staffs, axes, spears, guns, holsters, ropes, lassos, or
  props unless the target-specific identity explicitly allows them.
- Hero 3 boxer variants are the current exception: boxing gloves replace visible
  hands and are part of the outfit.

## Body Silhouette Rules

Male Chad variants:

- Exaggerated giga-chad heroic proportions.
- Extremely broad shoulders, huge chest, huge upper arms and forearms, narrow
  V-taper waist, large thighs, and clear thigh separation.
- Waist reads visibly thinner than the ribcage and chest.
- Clothing frames the narrow waist instead of creating a rectangular torso.
- Outfit surface may show readable chest, abs, serratus or oblique planes,
  biceps, forearms, quads, and narrow waist through broad clean contour planes.

Female Stacy and companion variants:

- Adult stylized athletic fighting-game proportions.
- Very large stylized bust, narrow waist, wider hips, and stronger/larger
  thighs.
- Bust, waist, hips, thigh mass, and athletic torso planes should read through
  fitted outfit surfaces.
- Larger bust and hips must come from silhouette and broad body planes, not from
  extra straps, trim, jewelry, exposed lingerie, or pin-up posing.
- Keep the clear vertical white leg gap for rigging.

Avoid childlike, tiny, soft, generic slim anime silhouettes, or sexualized
lingerie/pin-up presentation.

## Hero Identity Rules

Generate one hero variant per prompt. Mention only the active target identity.

- Hero 1: George Washington-inspired founding-era officer.
- Hero 2: Lu Bu-inspired ancient warrior.
- Hero 3: African American boxer-inspired fighter.
- Hero 4: Billy/cowboy-inspired western fighter.
- Hero 5: Yakub-inspired mystic/scientist identity.

Pairing rules:

- Each hero's Chad and Stacy variants share the same ancestry, skin-tone family,
  hair-color family, and theme identity unless Pablo explicitly changes it.
- Hero 3 Chad and Stacy are both African American boxer variants.
- Hero 5 Chad and Stacy both need a Yakub-inspired classic head shape: oversized
  wide cranium, broad flared temple lobes, high domed forehead,
  heart/spade-like upper head silhouette with a slight top-center dip, narrow
  lower face and jaw, and visible forehead mass. White hair frames the outer
  sides of the head shape. Keep it dignified and non-caricatured.

## Demo Clothing Rules

Use these rules for demo skins, demo outfits, birthday/dunce overlay tests, and
temporary demo roster images.

- Keep the character's base identity readable, but simplify the outfit into
  broad, clean sections.
- Add one bright cone dunce demo hat.
- The demo hat should be a saturated yellow, orange, magenta, or other clearly
  bright color that separates from skin, hair, and the source background.
- The demo hat must be blank: no letters, symbols, markings, icons, stripes,
  patterns, or text on the hat.
- If the base identity has a hat or headwear, replace that hat with the dunce
  demo hat. Do not stack the demo hat on top of a tricorn, cowboy hat, cap,
  helmet, crown, or other original hat.
- If the base identity has a plume or crest, only keep it when it does not merge
  with the demo hat or hair.
- Add one diagonal demo sash across the torso with large readable uppercase text:
  `DEMO`.
- The only visible text should be `DEMO`.
- Keep the sash as one large simple color region. Do not add confetti, tiny
  icons, tiny badges, party clutter, balloons, ribbons, or patterned noise.
- Avoid jewelry, earrings, necklaces, badges, buttons, and other small extras in
  demo variants unless Pablo explicitly asks for them.
- Demo clothing must keep the locked humanoid pose, foot rules, thigh gap, and
  silhouette rules.
- No weapons or non-outfit props in demo images. Hero 3 boxing gloves remain
  allowed.

## Batch Generation And Output Rules

- Generate one target identity per live prompt. Do not ask one image-generation
  call to create multiple distinct characters.
- When running multiple Codex CLI workers, each worker must save to exact target
  filenames and verify each file opens before reporting success.
- Do not copy "the newest image" from the shared generated-images folder when
  multiple workers are running in parallel. Use the session-specific generated
  path, a repo wrapper that saves exact names, or serialize copying inside each
  worker.
- Prefer accepted source candidates on a portrait canvas, preferably
  `1024x1536`, with the full body visible and centered. Do not crop or repair
  an accepted source just to change the background.
- Keep original selected outputs only when the nonstandard dimensions are
  intentionally part of a test. For model-generation staging, use the normalized
  portrait canvas.

## Companion Identity Rules

Current demo companion identities:

- Rap vixen: adult lightskin Black female companion, modern music-video/street
  fashion energy, fully clothed and non-explicit.
- Bar maiden: adult blonde medieval tavern-host companion, warm confident
  energy, fully clothed and non-explicit.
- College brunette: adult brunette campus/college companion, casual modern
  outfit, fully clothed and non-explicit.
- Office lady: adult black-haired office/professional companion, simplified
  work outfit, fully clothed and non-explicit.

Companions use the same locked humanoid pose, mature face style, color
discipline, and Stacy-style adult athletic silhouette rules. Regular companion
outfits should not include the demo hat or sash. Demo companion outfits must use
the demo clothing rules above.

## Review Gate

Reject or iterate if any of these occur:

- feet angle outward
- boot side profiles are visible
- stance becomes wide, heroic, or action-oriented
- arms touch the body instead of holding the A-pose 1.5 gap
- thighs, knees, calves, or boots touch
- character turns three-quarter or crops out body parts
- outfit hides the body silhouette
- male variant lacks broad-upper-body/narrow-waist V-taper
- female variant lacks very-large-bust/wider-hips/stronger-thigh read
- Chad and Stacy variants of the same hero have mismatched outfit complexity
- colors mix randomly across garment sections
- fine details or tiny ornaments dominate the design
- below-waist clothing reaches the knees or lower
- face becomes soft cartoon, chibi, or simplified anime
- demo images stack a dunce hat on top of an original hat
- demo hats contain text, stripes, icons, or markings
- demo images omit the `DEMO` sash
- demo variants add jewelry, earrings, necklaces, badges, buttons, or other
  small extras that were not explicitly requested
- weapons or props appear outside explicit target exceptions
- source is not full-body or subject/background separation is unreadable
