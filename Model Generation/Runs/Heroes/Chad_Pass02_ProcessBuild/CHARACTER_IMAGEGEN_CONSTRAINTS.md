# Character ImageGen Constraints

These constraints apply before any Chad source image is accepted for TRELLIS.
They are stricter than normal concept-art prompts because TRELLIS will turn
image-generation mistakes into geometry.

## Global Source Rules

- One subject only.
- Centered orthographic production reference.
- Pure flat opaque white `#ffffff` background.
- No alpha.
- No floor.
- No cast shadow.
- No contact shadow.
- No reflection.
- No gradient.
- No border card, poster panel, source sheet, or UI frame.
- No text, watermark, logo, signature, or labels.

## Body Rules

- Body-only means neck-down only.
- No head, face, hair, ears, beard, chin, skull fragment, hat, or mask.
- The top of the body must have a clean open circular neck socket above the
  trapezius, ready for a separate head and neck insert.
- If the outfit has a vest, jacket, armor collar, scarf, or shoulder wrap, the
  center neck area must stay open. Do not let clothing form a high collar,
  turtleneck rim, blue cloth wall, or raised back flap that reaches the ears.
- The neck socket must be intentionally visible from the side as a recessed
  opening for a neck, not a closed collar cap. Avoid body sources where the
  head would have to be pushed down into clothing to hide the join.
- Feet must point straight forward.
- Feet must be symmetrical: same angle, same size, same stance width, same
  grounding height.
- Do not generate turned-out feet, pigeon-toed feet, one sideways foot, or one
  foot larger than the other.
- Feet should be parallel and pointed straight toward the viewer, not
  near-parallel, not ballet turnout, and not wide outward angles.
- If the toes visibly angle left/right, reject the image.
- Arms should sit slightly away from the torso so the lats, waist, and arm
  silhouette are readable.
- Legs should be separated enough to avoid fused geometry.
- No weapon, shield, prop, cape, long coat, robe, or below-knee hanging cloth.

## Body Calibration Baseline

Attempt05 is the current good-enough calibration direction for Chad body prompts:

- use concrete silhouette ratios instead of only saying "more muscular"
- describe the torso as an exaggerated inverted triangle
- force shoulder width to be at least `3.5x` waist width
- force chest/ribcage width to be at least `2.5x` waist width
- tell the model to add mass only to traps, chest, shoulders, lats, arms, and
  legs
- explicitly forbid thickening the waist or abdomen
- keep the outfit open around the chest and waist so the V taper remains visible
- describe front-view boots as rounded rectangles facing the camera

The reusable body prompt should include this wording:

```text
Silhouette: exaggerated inverted-triangle torso. Shoulder width is at least
3.5x the waist width. Chest and ribcage width are at least 2.5x the waist
width. Enormous traps and chest, huge lats flaring outward, tiny pinched wasp
waist, narrow belt line, narrow hips. Add mass only to traps, chest, shoulders,
lats, arms, and legs; do not thicken the waist or abdomen.
```

And this feet wording:

```text
Feet/boots: strict front-view boots. Both toe caps face directly toward the
camera like two matching rounded rectangles. Shins are vertical, knees face
forward, ankles sit directly below knees. Feet are parallel, same size, same
angle, and same height on the canvas. No duck-foot stance, no outward turnout,
no side-profile boots, no visible side soles, no diagonal toes.
```

## Chad Anatomy Rule

If the body could plausibly be a realistic bodybuilder, it is not Chad enough.

The target is a stylized giga-chad/nephilim body:

- impossible heroic upper-body mass, larger than any previous calibration image
- enormous traps
- extremely broad shoulders
- oversized deltoids
- massive oversized barrel chest
- chest and traps should be the largest visual read in the silhouette
- upper body larger than the previous accepted-looking calibration image
- thick arms and forearms
- huge quads and calves
- very thin wasp waist, thinner than the last calibration image and clearly
  narrower than the ribcage and hips
- extreme V taper
- human-readable but beyond human realism

The upper body should be pushed further than realistic anatomy. The waist should
be thinner than realistic anatomy. Do not settle for athletic, lean, fashion,
fitness, or normal superhero proportions.

## Clothing Rules

Clothing must read as stylized cartoon/cel-shaded game texture, not realistic
cloth simulation.

The lighting must be bright and cartoony:

- high-key cel shading
- simple hard-edged stylized shadows only on the character surface
- no ground shadows
- no soft realistic studio lighting
- no dark moody rendering
- no heavy ambient occlusion
- no realistic cloth folds that create muddy shadow bands

Reject body images with:

- bra-like chest straps
- sports-bra shapes
- corset shapes
- bondage harness shapes
- random chest bands
- underwear-like tops
- ambiguous cropped chest garments
- straps that visually split the pecs like a bra

Outfits should be simple, readable, and character-specific:

- open sleeveless vest
- torn gi or martial wrap
- shoulder armor plates
- fitted leather/cloth panels
- belt and boots
- wrist wraps or bracers

The chest silhouette should stay dominated by the muscular torso, not by garment
straps.

## Head Rules

- Head-only means head plus full neck only.
- No shoulders, torso, shirt, armor collar, necklace, scarf, or clothing.
- No traps, trapezius slope, shoulder caps, collarbone shelf, or upper chest.
- Bottom of the image should show only a narrow insertable neck plug, not the
  top of the body.
- The neck must not be a full-width rectangular column. The visible neck should
  be narrower than the jaw, should taper under the mandible, and should stay
  centered.
- Reject any head source where the neck sides continue straight down from under
  the ears to the bottom of the canvas. TRELLIS turns that into a rear head/neck
  slab.
- Preferred head source shape: full skull and jaw, then a simple centered neck
  plug below the jaw. The neck plug should be roughly `25%` to `35%` of the
  head width at the lower edge of the source, wide enough to insert into the
  body socket but not wide enough to read as shoulders or a body column.
- Face axis must be centered and symmetrical.
- Eyes look directly at camera.
- No three-quarter turn, side tilt, looking left, or looking right.
- Skull geometry should stay consistent across the roster:
  large square cranium, broad brow ridge, high cheekbones, enormous square jaw,
  and thick neck.
- Hero identity comes from skin tone, ethnicity cues, hair color, hairline, hair
  silhouette, beard pattern, scars, makeup, and expression, not from shrinking or
  warping the Chad skull.
- Bright stylized cartoon/cel-shaded style, matching the accepted body
  direction.
- Flat front-facing cartoon light. Both sides of the face must be evenly lit and
  equally bright.
- Only minimal two-tone cel shading inside facial features.
- No one-sided face shadow, cheek shadow, jaw shadow, neck shadow, dark side of
  face, rim light, studio lighting, or ambient occlusion.
- Trademark giga-chad jawline: enormous square jaw, wide chin plane, heavy brow,
  high cheekbones, thick neck.
- Extreme block-jaw silhouette: jaw width nearly as wide as the cheekbones, wide
  flat horizontal chin plane, low square mandible corners, almost vertical jaw
  sides before the chin turn.
- Head should feel oversized and iconic enough for a top-down hero-select read,
  but not a caricature bobblehead.
- No realistic portrait lighting, skin pores, celebrity likeness, or soft photo
  rendering.
- No normal handsome male face, oval face, narrow chin, pointed chin, soft jaw,
  rounded jaw, fashion-model jaw, or one-sided shadow.

The reusable head prompt should include this wording:

```text
Skull silhouette: trademark giga-chad head geometry. Large square cranium, heavy
brow ridge, high cheekbones, thick muscular neck. Jaw silhouette: extreme
giga-chad block jaw. Jaw width is nearly as wide as the cheekbones. Chin is a
wide flat horizontal plane, not pointed or rounded. Mandible corners are large,
square, and low. Jaw sides are almost vertical before turning sharply into the
chin. The lower third of the face is oversized and powerful. Keep this skull
block consistent across every Chad hero; identity changes only through skin
tone, ethnicity cues, eye shape, hair color, hairline, hair silhouette, beard
pattern, scars, and expression.
```

## Head Calibration Baseline

Mike head Attempt03 is the current good-enough calibration direction for Chad
head prompts:

- head plus narrow insertable neck plug only
- no traps, shoulders, collarbone shelf, upper chest, clothing, or collar
- bright cartoon/cel-shaded style
- flat front-facing light
- both sides of the face evenly lit
- minimal two-tone facial shading only
- no one-sided face shadow
- extreme giga-chad block jaw
- jaw width nearly as wide as cheekbones
- wide flat horizontal chin plane
- low square mandible corners
- thick neck, but not a full-width rectangular column
- neck sides taper inward below the jaw and end as a centered plug

For future head generation, use Attempt03 as the baseline and change only the
identity details per hero. After Mike TRELLIS testing, the head source must also
pass a visual neck-width gate before TRELLIS: if the bottom neck is nearly as
wide as the jaw or continues down from the ears, preprocess or reroll before
model generation.

Mike A03/B02 is the current best head/body connection evidence. A03 improved
the side and rear neck plug compared with A02, but it also made the lower
face/neck slightly too triangular. Future prompts should not ask for a
triangular neck. They should ask for a giga-chad block jaw with a separate
centered neck plug that narrows below the mandible and stays visibly distinct
from shoulders, traps, or a full-width body column.

Mike A04 added the missing rear-hair lesson. Front-only head sources still need
to visually imply a complete hair mass around the skull, otherwise TRELLIS can
infer a bare crown or rear scalp patch. Future head prompts must include:

- full dense hair wrapping continuously around the skull
- hair visible along both side edges of the head
- rounded/full back-side hair mass, not only angular front spikes
- no scalp showing at crown, top, side edge, or inferred rear
- no bald spot, shaved crown, undercut patch, or skin-colored opening in the
  hair mass

Attempt04 solved the obvious rear bald spot after TRELLIS, but the generated
neck was too long and had to be buried during assembly. That means future
prompts should keep the full-hair language from A04 while tightening the neck
language from A03: visible real neck, short enough to be a plug, tapering below
the jaw, and not a tall rectangular column.

The Mike rig prototype and wider-neck comparison added the current neck-join
lesson after the rejected full-hair buried-neck attempt: the front read is best
when the A03-quality face is preserved, the head is lifted slightly above the
collar, and the neck volume is present behind and below the jaw. This should be
solved in the generated source, not by a flat procedural bridge. Future head
prompts should ask for a short, thick, textured giga-chad neck that continues
naturally under the jaw and remains visible from the side, while future body
prompts should leave a real open socket/collar gap for that neck. Do not solve
this by generating a long neck tower and burying it.

## Immediate Rejection Questions

Ask these before saving a source image as approved:

- Does the body look too realistic? If yes, reroll with more giga-chad mass.
- Are both feet pointing straight forward and symmetrical? If no, reject.
- Does the clothing form a bra, corset, harness, or random chest band? If yes,
  reject.
- Is the waist thin enough to read as exaggerated stylized anatomy? If no,
  reroll.
- Is the background perfectly flat white with no floor/shadow/contact patch? If
  no, reject.
- Does a body image contain any head remnant? If yes, reject.
- Does a head image contain any shoulders or clothing? If yes, reject.
- Does a head image show only front hair spikes with weak side/back hair mass?
  If yes, reject or reroll before TRELLIS.
- Does a head image imply a bald crown, shaved top, exposed scalp, or undercut
  opening? If yes, reject.
- Is the head neck a long rectangular tower rather than a short tapered plug?
  If yes, preprocess only for a process test, but reroll for production input.
- Does the body clothing climb high enough to cover the neck from the side?
  If yes, reject or regenerate with a lower open collar/socket.
