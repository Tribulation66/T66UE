# UI Reconstruction Style Memory

This file records the active visual policy for frontend UI work.

## Primary Rule

Do not invent the taste for a screen when an approved reference image already exists.

The approved reference image is the artistic authority. The job is to reproduce it faithfully with production-safe runtime UI.

## Current Source Of Truth

The main-menu reference image is the seed for the new frontend UI family.

That reference should drive:

- button shape language
- top-bar structure
- panel framing
- title treatment
- icon style
- decorative materials

Future screens should inherit this shared language unless a different approved reference explicitly replaces it.

## What The Runtime Must Preserve

- real interactive widgets
- real localizable text
- reference-matched spacing and hierarchy
- explicit visual states for controls
- scalable assets suitable for multiple resolutions and aspect ratios

## What The Runtime Must Avoid

- flattened screenshot overlays as the shipped implementation
- invisible hotspot grids as the shipped interaction layer
- baked localizable text inside control textures
- reuse of retired exploration material or superseded taste boards as design guidance

## Reference-Derived Asset Policy

When reconstructing a screen, derive reusable assets from the reference by family:

- top bar
- CTA buttons
- tabs and dropdowns
- left and right panels
- icon buttons
- separators and decorative pieces

Preferred generation order:

1. lock the approved reference
2. generate or reconstruct family boards from that reference
3. slice or export runtime-ready assets from those boards
4. rebuild the screen with real widgets and live text
5. validate with a live screenshot

For controls with text, the preferred runtime form is:

- empty plate art
- live text layered in-engine
- state variants for hover and pressed behavior
- coordinate-driven placement from the screen's reference-layout manifest

## Text Policy

Match the reference with real runtime typography.

Required:

- all major labels use real `FText`
- the chosen Latin display and UI fonts must visually match the reference closely
- localization support takes precedence over convenience

## Validation Policy

Every serious pass must end with:

1. a live screenshot
2. a comparison against the approved reference
3. a concrete mismatch list
4. the next correction pass

If the mismatch is visible, it is still part of the work.

## Retirement Policy

Retired visual directions, old mockup families, and superseded style assets should not remain as active guidance once a reconstruction-first replacement exists. Keep neutral plumbing. Remove artistic bias.
