# LootCrate Animation Mechanism Packet

**Created:** 2026-05-26
**Target:** LootCrate post-interaction UI animation
**Owning source:** `Source/T66/UI/T66CrateOverlayWidget.*`
**Archetype:** `ScrollingStripReveal`

## 1. Source Evidence

Primary process reference:

- `C:\Users\DoPra\Downloads\NoteGPT_TRANSCRIPT_CSGO CrateCase Opening System  Part 1 - Unreal Engine Tutorial (1).txt`
- `C:\Users\DoPra\Downloads\NoteGPT_TRANSCRIPT_CSGO CrateCase Opening System  Part 2 - Unreal Engine Tutorial (1).txt`

Part 1 evidence:

- Build a widget containing a horizontal item strip.
- Populate the strip with item images.
- Move images into an array, clear the strip, shuffle the array, then add the images back.
- Recheck for remaining children because the first clear pass may not empty the horizontal box.

Part 2 evidence:

- Add an open gate so the crate is opened once per sequence.
- Move the horizontal box through widget animation translation.
- Use repeated fast slide passes, then slower passes, then a final stop animation.
- Duplicate enough images/layout capacity so the strip does not expose empty space.
- Reset before each new open so the layout reshuffles.

Supporting written references:

- Unreal Garden UI Animation: supports track/easing/event mechanics but is not the primary crate process.
- Epic forum "Slotmachine" image-wheel thread: confirms the CSGO-inspired image-wheel problem shape but does not provide a finished implementation process.

## 2. Artifact Parity

ARTIFACT PARITY GATE

Reference artifact/category: CS:GO-style horizontal item strip crate opener.

Role: Primary

Required: YES

Planned artifact/path: `Source/T66/UI/T66CrateOverlayWidget.*` plus shared strip math in `Source/T66/UI/Animation/T66LootUIAnimation.*`

Status: SAME

Evidence: T66 uses a data-driven strip, randomized decoys, a locked winner, a fixed selector, scrolling/deceleration/settle phases, reveal markers, and reward commit markers.

## 3. Mechanism Manifest

MECHANISM MANIFEST

Reference/source: Matt Aspland CS:GO Crate/Case Opening System transcript Parts 1 and 2.

Required mechanisms:

1. Mechanism: item strip population and shuffle/reset
   Required: YES
   Planned implementation: Generate T66 item strip entries from live item data with randomized decoys and a locked winner for each overlay open.
   Evidence needed: source inspection and capture showing a populated strip.

2. Mechanism: enough offscreen strip length
   Required: YES
   Planned implementation: Derive final/fast/overshoot offsets from strip width, selector position, item stride, winner center, and explicit tuning.
   Evidence needed: capture frames showing no empty strip gap during travel.

3. Mechanism: fixed selector with moving strip
   Required: YES
   Planned implementation: Keep the selector centered while the strip container render transform moves.
   Evidence needed: multi-frame capture showing stationary selector and moving items.

4. Mechanism: staged fast-to-slow motion and final stop
   Required: YES
   Planned implementation: Drive native timelines for anticipation, fast travel, weighted deceleration, overshoot, and settle.
   Evidence needed: contact sheet or frame range showing fast-to-slow travel and final landing.

5. Mechanism: landing/reveal/commit
   Required: YES
   Planned implementation: Preserve landing, rarity reveal, and inventory commit markers after the selected item lands.
   Evidence needed: source inspection and runtime capture/log proof.

6. Mechanism: reset/skip gating
   Required: YES
   Planned implementation: Preserve `RequestSkip`, terminal state handling, and marker-driven reward commit.
   Evidence needed: source inspection; manual skip capture may be added if regression appears.

## 4. Current Revalidation

The old pending issue "Crate Overlay Header And Source Are Mid-Migration" was revalidated on 2026-05-26 and found stale.

Evidence:

- `RebuildWidget()` calls `BuildFullAnimationSequence()`.
- `TickAnimation()` drives `FT66AnimationSequence`.
- `BuildSkipToSettleSequence()` reads the same `StripState` offset fields as the full sequence.
- Header inspection found no legacy scroll timer or compatibility fields.

## 5. Anti-Lookalike

Cheapest wrong result: a static item card, or a strip that jumps directly to the winning item without continuous travel.

Discriminator: Unreal-owned video must show a fixed selector, moving strip, no empty strip gap, fast-to-slow travel, final stop, and reveal/commit after landing.

## 6. Verification Plan

- Focused `T66Editor Win64 Development` build.
- Isolated `lootcrate` capture through `Scripts/CaptureT66GameplayVideo.ps1`.
- ffprobe duration and frame count.
- Contact sheet or frame-range evidence.
- Runtime log confirming `[LootUICapture] lootcrate triggered`.
- Mechanism close for the required mechanisms above.
