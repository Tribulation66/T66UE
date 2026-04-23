# Screen Review Gate

Use this gate before calling a reconstruction pass successful.

## Why This Exists

A screen can look closer in one region while regressing badly in another. This gate exists to stop churn and stop low-signal "improvements" from being accepted.

## Mechanical Checks

Every pass must clear all of these:

- no cropped text
- no cropped component edges
- no duplicated shell ownership
- no shell art extending outside its measured frame
- no dark placeholder blocks where a plate or environment strip should be
- no stretched plate art caused by forcing the wrong aspect ratio into a manifest box
- no top-bar, panel, or CTA region that is simultaneously owned by both the background and live widgets
- no obvious live-control dead zones or fake visible buttons

## Reconstruction Checks

For a reference-faithful screen:

- reference layout manifest is the placement source of truth
- recurring chrome uses clean runtime families, not screenshot cleanup hacks
- localizable text is not baked into runtime art
- packaged runtime is used for the actual judgment
- static regions are diffed strictly when possible
- live regions are called out explicitly when they were only manually validated

## Review Process

1. Capture the packaged screen.
2. Compare against the approved reference at the same composition.
3. List ownership/layout failures before discussing polish.
4. Fix one region at a time.
5. Roll back immediately if a pass improves one region by breaking another.

## Status Language

Use blunt labels:

- `blocked`
- `close with blockers`
- `exact enough`

Do not call a pass final if it still fails any mechanical check.
