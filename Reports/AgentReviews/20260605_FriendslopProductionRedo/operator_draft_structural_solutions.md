# Operator Draft: FriendslopStyle Structural Solutions

Short answer: the clean alpha sheet is inadequate as a source for Round06. The solution is not another blind sheet attempt; the next production pass has to be blocked until the plate-authoring method, panel safe areas, and reference-category gates are changed.

## 1. Split / missing-middle line problem

Cause:
The split line is a source-and-slicing failure. Several plates are being treated like resizable 9-slice/box brushes even though their source art does not contain a continuous neutral center band. The preserved top and bottom cap detail survive, but the middle reads as a groove or missing strip. That is why the button looks like two halves, not one inflated rubber surface.

Actual solution:
Do not vertically 9-slice those controls. For stable-height buttons and fields, use either fixed-size plates or horizontal 3-slice plates: fixed height, stretch only width. If any element must resize vertically, the source plate must be re-authored with a real continuous center fill before it can be sliced. A plate with a built-in central groove is not valid for this target, regardless of margin tuning.

Acceptance gate:
Before Unreal import, every plate family gets a min/normal/wide contact sheet at target runtime height. It fails if a horizontal seam, separated top/bottom caps, clipped glow/shadow, smeared bevel, or missing center band appears. In the next PPF/artifact gate, these controls should be marked fixed-size or horizontal-slice unless a clean vertical stretch band is proven.

## 2. Content fitting inside panels

Cause:
The rows and controls were allowed to be "close" in absolute screen coordinates without a strict parent content rectangle. That lets rows, labels, buttons, and table content crowd or escape the actual usable body inside the decorative panel.

Actual solution:
Each panel must have two boxes: the outer chrome bounds and an explicit safe content rect/inset. All children are laid out relative to that safe rect, clipped by it, and verified against it. Friend rows, search fields, headers, party slots, leaderboard dropdowns, table headers, and ranking rows need fixed row heights, fixed gaps, max widths, and overflow behavior. Lists scroll or clip inside the body; they do not resize the panel and do not paint into the border.

Acceptance gate:
The checklist/dump must include `contained_in=<parent> inset=<l,t,r,b>` for every row/control that lives inside a panel. Add stress fixtures, not just the reference fixture: longest friend names, max online/offline counts, long leaderboard names, empty/loading states, and localized strings. A pass is only valid when both fixture and stress captures show no escape, overlap, or border collision.

## 3. Reference mismatch problem

Cause:
The source family is categorically wrong. The pass10 plates are generic pipe/groove rubber atoms. Round06 uses smoother inflated panels and pills with subtler bevels, different region weight, and different button faces. Matching coordinates and sharing red/black/green colors cannot fix a wrong element class.

Actual solution:
Re-author the runtime chrome by reference category, not by generic atom reuse. The left panel shell, right leaderboard shell, search/header/friend row surfaces, CTA primary, CTA secondary, tabs, dropdowns, small icon buttons, and leaderboard rows each need their own reference-matched plate or plate family. Imagegen can create candidates, but Photoshop-equivalent cleanup/compositing is expected. Live Slate still owns text, names, icons, counts, scores, handlers, and states.

Acceptance gate:
Every new plate is shown beside its Round06 crop at target runtime size before it is accepted. The gate must ask "same element category?" first, before polish. If the side panel is still a pipe frame, or the central button is still a split rail/groove button, it fails even if geometry passes. Whole-screen acceptance requires the visual scorecard to pass first-glance match, silhouette/region weight, panel/button hierarchy, rubber material fidelity, authored plate quality, overflow/clipping, and live content preservation.

## Required process change before the next production pass

The next pass should start with a plate taxonomy and safe-rect specification, not generation:

1. List every required Round06 plate family and classify it as fixed-size, horizontal-slice, or proven 9-slice.
2. For panels, define outer chrome bounds and inner content safe rects.
3. Produce per-family reference-crop contact sheets before runtime integration.
4. Run seam/stretch gates before import, containment gates after layout, and holistic visual scorecard after capture.

The user decision needed before implementation resumes is whether we are allowed to prefer size-specific fixed plates for the CTAs and large panels when that is the only way to preserve the Round06 look. My recommendation is yes: for this screen, visual fidelity matters more than forcing a single resizable brush across elements that should not be vertically stretched.
