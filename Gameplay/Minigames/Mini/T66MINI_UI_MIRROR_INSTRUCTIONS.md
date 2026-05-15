# UI Mirror Process

## Purpose

Use this process whenever a mini pregame screen is being rebuilt from a visual reference. The goal is not "close enough." The goal is a screen that matches the reference layout and presentation exactly, unless a difference is explicitly approved.

## Inputs

- One or more reference images from the user.
- A note describing what is intentional vs. what may differ.
- The target screen name.

## Rules

1. Treat the reference as the source of truth for layout, hierarchy, spacing, sizing, alignment, and overall composition.
2. If the user wants a different background image or different art than the reference, keep the layout match exact and only allow the approved art swap.
3. New mini-specific assets and files should use a `mini_` prefix whenever we are duplicating an existing non-mini asset for future divergence.
4. If unrelated concurrent mini changes appear in the worktree, do not overwrite them. Wait or route around them.

## Execution Loop

1. Gather the reference images and write down the non-negotiable traits for the screen.
2. Implement the screen changes in code and assets.
3. Launch the game and open the exact target screen.
4. Capture a screenshot of the implemented screen.
5. Compare the screenshot against the reference and assign a percentage match.
6. List the visible gaps that still prevent a 100% match.
7. Do another implementation pass focused only on those gaps.
8. Repeat the capture and comparison loop until the screen is effectively identical to the reference layout.

## Match Scoring

Score against the reference using these categories:

- Layout match
- Element sizing
- Alignment and spacing
- Typography treatment
- Color and contrast treatment
- Visual hierarchy
- Motion or animated treatment, if relevant

The score should reflect what is visible on screen, not what the code intended to do.

## Done Criteria

The screen is only done when:

- The latest screenshot matches the reference layout exactly or as closely as physically possible in the engine.
- Any remaining differences are intentional and documented.
- The implementation has been reviewed against the screenshot, not just the code.

## Deliverables Per Screen

- Updated code and assets
- A latest in-engine screenshot
- A percentage match assessment
- A short list of what changed in the final pass
