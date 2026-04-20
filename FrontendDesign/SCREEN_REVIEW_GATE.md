# Screen Review Gate

Use this gate before presenting any discovery or production screen pass.

## Why This Exists

A screen can be interesting in taste and still fail badly in craft.

This gate exists to stop obvious presentation failures such as:

- text clipping
- component clipping
- accidental black margins around buttons
- off-center labels that read as mistakes
- title plaques showing the wrong background content
- panel fills leaking outside their frames
- control groups that are not internally aligned

If these failures exist, the pass is not ready to show as completed.

## Mechanical Checks

Every pass must clear all of these:

- no cropped text
- no cropped component edges
- no accidental dead space around active button plates
- no shell artwork larger than its intended frame
- no title treatment swallowing, colliding with, or mismatching the title
- no misaligned labels, icons, or control rows that read as unintentional
- no obvious contrast failure on primary actions
- no decorative content bleeding into the wrong surface

## Discovery-Specific Checks

For multi-family exploration:

- preserve the component map unless a structural change was approved
- do not reuse older generated variants as hidden inspiration
- track whether each major visual ingredient is canonical, repo-sourced, newly generated, or approved carryover
- make families separate in shell language, not only in color grade

## Production-Specific Checks

For the chosen direction:

- the screen must look coherent at full view and thumbnail view
- title, centerpiece, and CTA hierarchy must be immediately readable
- the material stack must feel authored rather than like basic CSS boxes
- the output should be credible as a final Unreal-facing visual spec

## Review Process

1. Render the live page to an image.
2. Inspect the image directly, not just the code.
3. List craft failures before discussing taste.
4. Fix the failures.
5. Only then present the screen as a real pass.

## Status Language

Use blunt labels:

- `useful exploration, not presentation-ready`
- `improved but still below bar`
- `strong enough to keep refining`
- `production-ready direction`

Do not call a pass final if it still fails the mechanical checks.
