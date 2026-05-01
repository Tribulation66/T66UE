# UI Memory

## 2026-04-30 - Master Asset Library Source Rule

- New or replacement UI elements for `SourceAssets/UI/MasterLibrary` must be generated with Codex-native image generation from the approved visual reference first.
- Hard ban: do not use Pillow/PIL anywhere in the master UI asset-library chrome pipeline, including scripts, Python helpers, alpha cleanup, matte removal, resizing, cropping, slicing, proofing, or small repairs.
- Do not use procedural scripts, Slate drawing, fill rectangles, crop-patch repainting, or scripted wood/metal/paper synthesis to invent, repaint, synthesize, or visually repair master-library buttons, panels, top bars, slots, paper, fill bars, dropdowns, select controls, or other chrome.
- Local scripts are allowed only after an approved imagegen board exists, and only for rectangular slicing, deterministic resizing that preserves pixels, copying, naming, manifests, alpha validation, and replacing already-background pixels outside the component silhouette with transparent black.
- Do not apply despill, edge cleanup, color mutation, repainting, sharpening, blurring, corner repair, or local pixel repair to master-library UI chrome.
- Any removed background pixel must be alpha 0 with RGB 0,0,0. Do not leave green, magenta, checkerboard gray, white, paper-colored, or other visible RGB in transparent pixels because Unreal filtering and mips can bleed it around button edges.
- Rounded, oval, chamfered, or otherwise non-rectangular controls must ship as alpha-bearing PNG slices with transparent corners; opaque rectangular matte/corner pixels or visible square backing layers are a hard rejection.
- Button labels render directly on the button surface. Do not add a separate rectangular label plate or brown square under text.
- For the main-menu wood family, basic buttons are rectangular dark-mahogany planks with thin antique-gold bevels and black pixel outlines. Invite and offline use the small rounded pill style with centered text. Basic panels include dark mahogany fill plus border, and paper background is only an inner content material.
- If the generated board is visually wrong, regenerate with imagegen instead of patching the pixels locally.
