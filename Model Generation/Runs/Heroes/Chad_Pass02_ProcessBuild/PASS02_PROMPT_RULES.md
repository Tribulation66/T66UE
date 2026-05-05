# Chad Pass02 Prompt And QA Rules

## Required Body Prompt Target

- one centered front-facing body only
- neck-down only: no head, no face, no hair, no ears, no beard
- open circular neck socket or clean neck hole above the trapezius, visible from
  the front
- arms slightly away from torso so the waist and lats are readable
- legs separated enough that TRELLIS does not fuse them
- no weapon, shield, prop, pedestal, back view, side view, duplicate figure, or
  character sheet panel
- both feet pointing straight forward at the viewer, perfectly symmetrical, same
  size, same stance angle, same grounding height, parallel rather than
  near-parallel or outward-turned
- giga-chad/nephilim proportions: impossible heroic muscle mass, huge traps,
  boulder shoulders, massive chest, thick arms, thick forearms, huge quads and
  calves, very thin waist, extreme V taper
- portrait canvas preferred: `1024x1536`

## Required Head Prompt Target

- one centered straight-on head only
- head plus full neck only
- no shoulders, no clothing, no torso, no necklace, no armor collar, no hat rim
  touching the crop unless the hat is essential identity and clearly isolated
- no traps, no trapezius slope, no shoulder caps, no collarbone shelf, no upper
  chest; bottom crop is only a straight neck cylinder
- standardized giga-chad skull block: large square cranium, broad brow ridge,
  high cheekbones, enormous square jaw, wide flat chin plane, thick neck, same
  skull scale across heroes
- jaw width nearly as wide as cheekbones; low square mandible corners; almost
  vertical jaw sides before a sharp turn into the chin
- hero identity only through skin tone, ethnicity cues, eye shape, hair color,
  hair silhouette, beard pattern, scars, makeup, and expression
- no skull-size drift except deliberate non-human cases such as Robo
- bright stylized cartoon/cel-shaded style matching the body source images
- no realistic portrait lighting, no celebrity likeness, no skin pore detail
- flat front-facing cartoon light; both sides of the face evenly lit and equally
  bright
- minimal two-tone cel shading only; no one-sided face shadow, cheek shadow, jaw
  shadow, neck shadow, rim light, studio lighting, or ambient occlusion

Current locked calibration: Mike head Attempt03 is good enough for the head
prompt baseline. Future head prompts should preserve its crop, flat front
lighting, bright cel-shaded style, and block-jaw language, changing only
hero-specific identity details.

Current TRELLIS/assembly calibration: Mike A03/B02 is the best head/body
connection evidence. The reusable target is not a narrow triangular head. It is
a full giga-chad block jaw with a centered, insertable neck plug below it. The
neck plug must be narrower than the jaw, must taper under the mandible, and
must not continue straight down from the ears or imply shoulders/traps.

## Background Rule

For Pass02, use opaque flat-white backgrounds:

- exact visual intent: `#ffffff`
- no alpha
- no floor
- no cast shadow
- no contact shadow under feet
- no reflection
- no gradient
- no gray patch, white panel, border card, or poster sheet

Green remains the historical reproduction baseline, but Batch01 showed that
green spill inside hair, beard, face, and clothing is dangerous. White is the
Pass02 experiment only if it stays perfectly flat and shadowless.

## Immediate Rejection Gates Before TRELLIS

Reject the source image before TRELLIS if any of these are true:

- body canvas is square unless a human reviewer explicitly approves it
- body has any head, face, hair, ears, or neck stump that reads like a generated
  head remnant
- head has shoulders, clothing, torso, armor collar, or crop edge contact
- any floor strip, cast shadow, contact shadow, platform, reflection, or foot
  grounding patch is visible
- any source panel, card, poster boundary, duplicate body, front/back sheet, or
  side view is visible
- background is not a single flat color at the border
- internal green, gray, or white background spill appears inside hair, beard,
  face, clothing, hands, boots, or between fused parts
- silhouette is merely athletic instead of giga-chad/nephilim
- feet are turned sideways, asymmetrical, pigeon-toed, or mismatched in scale
- feet point outward too much instead of forward
- toes visibly angle left or right instead of pointing straight at camera
- clothing reads as a bra, corset, harness, sports-bra top, underwear top, or
  random chest strap
- lighting is dark, realistic, heavily shadowed, or creates any floor/contact
  shadow
- skull/head size drifts away from the shared Chad skull target

## TRELLIS Gate After Generation

Keep the locked TRELLIS settings until the source gate is clean:

- seed: `1337`
- texture: `2048`
- body decimation: `80000`
- head decimation: `120000`

Before assembly, reject or reroll any raw body GLB whose side view is a vertical
card or whose measured depth is under roughly `0.15m`. Clean Batch01 bodies were
closer to `0.24m-0.31m` depth; bad poster bodies were near-flat.

## Review Order

1. Source PNG visual contact sheet.
2. Source PNG audit JSON.
3. Raw TRELLIS body/head front and side renders.
4. Raw GLB bounds/depth report.
5. Head/body assembly close neck views from front, side, and oblique.
6. Blender rig/import checks only after source and raw GLB quality pass.
7. Unreal data and staged executable visual verification only after import/wiring
   is intentionally redone.
