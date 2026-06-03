# LootWheel Animation Mechanism Packet

**Created:** 2026-05-26
**Target:** LootWheel post-interaction UI animation
**Archetype:** RadialWheelReveal
**Status:** Implementation packet

## Source Evidence

Primary reference:

- `C:\Users\DoPra\Downloads\NoteGPT_TRANSCRIPT_UI Fortune Wheel - UEFN Tutorial 07.txt`

Observed mechanisms:

- UI wheel appears after player interaction.
- Radial wheel surface rotates while a fixed pointer stays still.
- Rotation is driven by a normalized wheel-rotation value.
- Spin has a constant-speed phase.
- Stop phase decelerates before landing.
- Final fractional rotation maps to a segment index and result.

T66 adaptation:

- The reference lets final rotation decide the result.
- T66 locks the reward first in `AT66LootWheelInteractable`.
- Therefore T66 computes the final stop angle from the locked reward segment, then commits after landing.

Visual reference update:

- `C:\UE\T66\UI\Screen References\LootWheel_WheelOfFortuneReference.png`
- Source attribution: `C:\UE\T66\UI\Screen References\LootWheel_WheelOfFortuneReference.attribution.md`
- User direction: the first sparse Slate pass looked wrong; the wheel needs to read like a dense Wheel of Fortune style prize wheel.
- Observed visual mechanisms: many narrow wedges, high-contrast alternating colors, a dark rim, repeated rim marks, center medallion, and a fixed top pointer.
- Accepted deltas: T66 does not copy exact Wheel of Fortune brand text, exact dollar values, exact typography, exact licensed layout, or photographed material. T66 uses reward-class labels, a clean center medallion, and procedural Slate drawing.

Secondary reference:

- `C:\Users\DoPra\Downloads\NoteGPT_TRANSCRIPT_Simple Math Wheel Spin Unreal Tutorial.txt`

Use only as support for wheel rotation vocabulary. It is not the LootWheel UI reward process.

## Values

- Carrier: procedural Slate radial wheel, user-approved as `EQUIVALENT` to the reference's masked UI material/texture wheel carrier.
- Reward authority: locked reward from `AT66LootWheelInteractable`.
- Segment density: 24 wedges (`observed` as dense two-dozen-like reference layout; exact 24 count is `tuned` for Slate readability).
- Palette: bright alternating prize-wheel colors (`observed` from the stored reference, `tuned` to T66 procedural Slate).
- Center medallion: clean fixed overlay cover without text (`tuned` after user requested removal of the top `V` badge and center black stroke).
- Segment mapping: class-level Gold, Item, Boost segments; exact payload details are shown by the wheel result text and downstream handoff.
- Boost timing: user-approved landing-time commit, so the 10-second boost duration starts when the wheel lands.

## Artifact Parity

Reference artifact/category:
Radial UI wheel with fixed pointer and segment-based result.

Role:
Primary

Required:
YES

Planned artifact/path:
`Source/T66/UI/T66LootWheelOverlayWidget.h/.cpp`

Status:
EQUIVALENT

Evidence:
The implemented carrier is a target-owned radial UI surface that rotates under a fixed pointer, decelerates, lands on the locked reward segment, and reveals/commits after landing. Procedural Slate rendering replaces the tutorial's masked UI material/texture carrier with explicit user approval.

Reference artifact/category:
Dense Wheel of Fortune style prize-wheel reference image with many colored wedges, dark rim, rim marks, center medallion, and fixed pointer.

Role:
Primary visual reference

Required:
YES

Planned artifact/path:
`UI/Screen References/LootWheel_WheelOfFortuneReference.png`

Status:
SAME reference artifact, EQUIVALENT runtime Slate implementation

Evidence:
The local reference image is stored in the repo with attribution. Runtime output is compared against the reference with a LootWheel-specific accepted-delta report.

## Mechanism Manifest

1. Mechanism: target-owned radial UI carrier appears after interaction.
   Required: YES
   Planned implementation: HUD-owned LootWheel overlay created by the presentation controller.
   Evidence needed: runtime log and MP4 frames showing LootWheel overlay after interaction/capture route.

2. Mechanism: radial wheel rotates beneath a fixed pointer.
   Required: YES
   Planned implementation: rotate `WheelRotationBox` while the narrow top selector and `ST66LootWheelFixedSelectorWidget` remain sibling overlays outside the rotating container.
   Evidence needed: multi-frame proof showing pointer/selector stay still and segments rotate.

3. Mechanism: clean wheel surface without tiling artifacts.
   Required: YES
   Planned implementation: procedural Slate segments; no texture tiling path; single parent rotation source so painted wedges and labels stay synchronized.
   Evidence needed: frames showing clean circular segment bounds during spin and no wedge/label desync.

4. Mechanism: constant-speed spin phase.
   Required: YES
   Planned implementation: timeline-driven linear spin through multiple rotations.
   Evidence needed: frame sequence with repeated radial motion before slowdown.

5. Mechanism: deceleration-to-stop phase.
   Required: YES
   Planned implementation: ease-out deceleration timeline ending at locked segment angle.
   Evidence needed: frame sequence showing decreasing angular travel per frame.

6. Mechanism: segment/result mapping.
   Required: YES
   Planned implementation: visible Gold, Item, and Boost segments in a 10:8:6 Gold:Item:Boost distribution across 24 wedges; locked reward class maps to an allowed segment index.
   Evidence needed: log/dump proving locked reward class equals landed segment class.

7. Mechanism: deterministic locked-reward landing.
   Required: YES
   Planned implementation: compute final angle from locked reward segment plus extra full turns.
   Evidence needed: log and capture showing final pointer over the locked class.

8. Mechanism: landing reveal and single commit/handoff marker.
   Required: YES
   Planned implementation: landing marker commits once, then overlay finish starts the existing downstream presentation or finishes interaction.
   Evidence needed: normal path and `EndPlay` fail-safe proof with no double grant.

9. Mechanism: skip/reset gating.
   Required: YES
   Planned implementation: skip moves to landing/reveal/dismiss without bypassing commit.
   Evidence needed: code evidence and runtime route sanity.

10. Mechanism: Unreal-owned capture and temporal validation.
    Required: YES
    Planned implementation: `lootwheel` gameplay capture mode and batch capture entry.
    Evidence needed: MP4, ffprobe duration/frame count, contact sheet or equivalent multi-frame proof.

11. Mechanism: reference-fidelity report.
    Required: YES
    Planned implementation: `UI/Checklists/lootwheel_reference_checklist.md` plus `Saved/Codex/UI/LootWheel/reference_fidelity_report.md`.
    Evidence needed: zero unresolved FAIL rows and documented accepted deltas against the stored reference image.

12. Mechanism: clean selector and medallion cleanup.
    Required: YES
    Planned implementation: remove the top `V` badge/pointer glow state, keep a narrow fixed selector above the medallion, and draw a clean fixed center medallion cover so no black stroke appears inside the yellow circle.
    Evidence needed: multi-frame MP4/contact sheet and staged capture showing no top `V` badge and no black stroke inside the center medallion.

## Anti-Lookalike

Cheapest wrong result:
A shared chest reward popup, pickup item card, static wheel image, actor-model wheel wiggle, or a radial wheel where the black outcome selector rotates with the wedges.

Discriminator:
The accepted result must show a target-owned radial UI wheel, fixed pointer, fixed black/gold outcome selector, moving radial segments, constant spin, deceleration, final stop on the locked reward class, and commit/handoff after the landing marker.
