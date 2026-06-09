# Codex Solution Draft

## Point 1 - Asset Method

Correction: the runtime did load PNG assets generated/prepared for FriendslopStyle; it was not pure Slate. But the user is right about the practical result. The assets were generic reusable blank chrome atoms, then Unreal stretched and arranged them. That is why the result reads like a manual Unreal reconstruction of the reference instead of the reference-quality UI.

Solution:

- Change FriendslopStyle from "generic reusable chrome sheet first" to "production-authored UI plates first."
- ImageGen/full reference remains vision-board input only.
- Runtime assets must be authored as transparent PNG plates for the actual UI element families on the screen: left panel plate, right leaderboard plate, CTA primary plate, secondary CTA plate, top bar strip/tab plates, row plate, dropdown plate, party slot plate, icon button plate.
- These plates can be generated, edited, composited, and cleaned in an external image-authoring stage. Photoshop-equivalent cleanup is allowed and expected. Unreal should not be responsible for creating the premium material look; Unreal should only render the final authored plates with live text/data on top.
- Reuse is allowed only where the reference actually supports it. High-detail rubber panels/buttons should have size-family variants instead of stretching one generic asset everywhere.
- 9-slice/3-slice is still useful, but only after a slice test proves the center stretch does not smear highlights, bevels, or shadows. If the slice test fails, make a size-specific plate.
- Add a hard asset-parity rule: every load-bearing surface in the reference must map to a specific runtime PNG plate or named accepted delta. "Looks similar to a generic rounded button" is not enough.

## Point 2 - Sizing And Fitting

Problem: the verifier checked individual widget existence and loose positions, but did not check parent-child containment or proportional scale. That allowed the right leaderboard row to be visibly too wide for the panel.

Solution:

- Add a layout-lock pass before visual polish: every major region and every repeated child family gets measured from the reference and converted into exact normalized boxes.
- Add containment assertions to the checklist/verifier:
  - child bounds must be inside parent bounds;
  - table row bounds must be inside table body bounds;
  - list rows must fit the list interior width;
  - row height and row gap must match the reference within a tight tolerance;
  - columns inside a row must not overlap or escape.
- Tighten geometry tolerances for load-bearing elements. Broad region tolerances like 0.020 are too loose for small rows and tabs.
- Add a special table/list contract for leaderboard and friends list:
  - panel outer bounds;
  - panel content inset;
  - header row bounds;
  - body clip bounds;
  - first row bounds;
  - max row width <= body width;
  - row x/y/w/h relative to body, not only screen absolute x/y.
- Make row asset dimensions derive from the reference row slot, not from the largest reusable row source texture.
- Responsive captures remain required, but they do not replace the 1920 reference-fit gate.

## Point 3 - Fidelity Assessment

Problem: both Codex and Claude let a structured `PASS=94 FAIL=0 UNSURE=0` stand in for visual fidelity. That was wrong. The verifier only proved that chosen assertions passed; it did not prove the screen looked like the reference.

Solution:

- Redefine DONE for FriendslopStyle:
  - structured dump checklist clean;
  - containment/overflow checklist clean;
  - visual reference-fidelity gate passed;
  - both Codex and Claude visually inspect the reference, capture, and contact sheet;
  - any one failed gate means `NEEDS_WORK`, not done.
- Add a mandatory visual scorecard before final:
  - first-glance match;
  - layout/silhouette match;
  - panel/button scale hierarchy;
  - rubber material fidelity;
  - authored asset quality;
  - text/content preservation;
  - overflow/clipping;
  - reference-specific deltas.
- Each category gets `PASS`, `FAIL`, or `USER_ACCEPTED_DELTA`. No category can be silently omitted.
- Claude validator must open the image artifacts and give an independent visual verdict before reading the PASS count. Validator cannot approve from report text alone.
- The final answer must not say "done" unless the visual scorecard passes. If the scorecard is below target, the result is either another iteration or an explicit blocker.
- Add a "cheapest wrong result" check for each screen. For this pass, the wrong result was: generic rubber-looking controls in approximately the right places. The discriminator should have been: the whole capture reads as the approved Round06 image at a glance, with matching scale, density, and panel/row relationships.

## Proposed One-Line Policy Change

For FriendslopStyle, `VerifyUIFidelity.py` is necessary but not sufficient. A screen is accepted only when the structured dump, containment checks, and independent visual scorecard all pass; otherwise the pass remains partial.
