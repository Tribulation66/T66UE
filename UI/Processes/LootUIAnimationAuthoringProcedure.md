# Loot UI Animation Authoring Procedure

**Created:** 2026-05-26
**Scope:** T66 post-interaction 2D/UI reward animations such as LootCrate, LootChest, LootBag, and LootWheel.
**Status:** Procedure and validation contract. Per-target packets own implementation values.

## 1. Ownership

This document owns the reusable process for loot UI animation work. `AGENTS.md` owns global PPF, artifact parity, mechanism manifests, review, and Unreal-owned capture gates. Per-target packets own concrete source evidence, tuned values, touched files, and acceptance results.


## 2. Source Evidence

For video references, use transcripts supplied by Pablo in the request or already stored in the named packet. Do not run ad hoc transcript extraction. Non-video written references should be read directly.

Each target packet must record:

- source/reference name and path or URL,
- transcript or written-source evidence used,
- values labeled `observed`, `inferred`, or `tuned`,
- primary animation archetype,
- artifact parity gate,
- mechanism manifest,
- anti-lookalike discriminator,
- Unreal-owned capture plan.

## 3. Archetypes

### ScrollingStripReveal

Use for CS:GO-style crate/case openers and any strip/roulette-strip UI that moves an item strip under a fixed selector. Do not use this archetype for the current LootWheel target; that target is a radial wheel.

Required mechanisms:

- item strip population from a reward candidate set,
- randomized or authored decoy order before each open,
- enough offscreen strip length for the full travel,
- fixed selector with moving strip,
- fast-to-slow travel and final stop,
- landing/reveal/commit gate after the selected item lands,
- reset/skip gating.

### RadialWheelReveal

Use for wheel-of-fortune style reward presentations where a radial wheel spins under a fixed pointer and lands on a reward segment. Current example: LootWheel.

Required mechanisms:

- target-owned radial UI surface after interaction,
- visible wheel segments mapped to reward classes or rewards,
- fixed pointer with rotating wheel surface,
- constant-speed spin phase,
- deceleration-to-stop phase,
- deterministic final angle derived from the locked reward,
- landing/reveal/commit gate after the locked segment lands,
- reset/skip gating,
- Unreal-owned temporal capture proof.

### ContainerOpenReveal

Use for chest, bag, or box-opening presentations where a container opens before the result card or item appears.

Required mechanisms:

- closed container state,
- opening motion or frame sequence,
- result emergence or result handoff,
- reveal timing,
- commit/handoff marker,
- no actor-model wiggle substituted for the UI animation.

### SelectionCardUI

Use for target interactions that are cards, selectors, or choice panels rather than live reveal animations. Current examples include IdolAltar and WeaponAltar. Do not report this as a live animation unless a target-owned temporal reveal is added.

## 4. Authoring Order

1. Select the archetype and source process.
2. Write or update the target mechanism packet.
3. Verify the live owning code path.
4. Build the smallest reusable infrastructure that the current target actually uses.
5. Implement the target-owned UI animation surface.
6. Preserve reward locking, commit, skip, and handoff semantics.
7. Capture with Unreal-owned tooling.
8. Validate temporal behavior with frame-range evidence.

## 5. Anti-Lookalike Rule

Name the cheapest wrong result before implementation. Common wrong results:

- a static reward card labeled as an opening animation,
- a strip that jumps directly to the winner,
- a static radial wheel that does not spin under a fixed pointer,
- a moving background with a static result,
- a shared chest/card presentation mislabeled as LootWheel or LootBag,
- a single still used as proof of temporal behavior.

The discriminator must prove the target-owned mechanism. For `ScrollingStripReveal`, proof requires a stationary selector, moving item strip, fast-to-slow travel, final stop, and reveal/commit after landing. For `RadialWheelReveal`, proof requires a stationary pointer, rotating radial segments, constant spin, deceleration, final stop on the locked reward segment, and reveal/commit after landing.

## 6. Capture And Validation

Use Unreal-owned capture paths from `AGENTS.md`.

Required evidence for full acceptance:

- focused build or stronger compile proof,
- runtime route log,
- MP4 path,
- ffprobe duration/frame count,
- contact sheet or equivalent multi-frame proof,
- visibility check for target size, occlusion, angle, and temporal sampling,
- mechanism close for every required mechanism.

Temporal mechanisms cannot be accepted from a single still.

## 7. Close Template

```text
LOOT UI ANIMATION CLOSE
Target:
Archetype:
Process used:
Artifact parity:
Mechanism close:
Anti-lookalike result:
Capture evidence:
Reported status: FULL/PARTIAL
```
