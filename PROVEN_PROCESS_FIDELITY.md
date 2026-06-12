# Proven Process Fidelity

Use this file only when a task is process-governed: solved-category visual work, animation, rigging, VFX, imports, UI fidelity, generated media, audio, or another task where QA cares how the result was produced rather than only what value changed.

Skip PPF for trivial tasks where QA would care only that the value changed, such as a single config value, typo, rename, or isolated data-row edit. Do not use this exemption for materials, meshes, particles, UI layout, animation, audio, staged builds, imports, or generated media.

## Rule

- If a task matches an accepted process, folder `*_AGENTS.md`, instruction doc, user-named workflow, tutorial/reference standard, or previously corrected failure pattern, that process is mandatory.
- Speed, simplicity, or confidence in a shortcut does not justify method substitution.
- Same toolchain is not enough. A simplified path is allowed without approval only when it keeps the same primary artifact roles and reduces scope, not method class.

## Research-First Replication

- First collect and reproduce a process; do not invent an approximation.
- Replication means method and mechanism fidelity. It does not mean copying unlicensed tutorial assets, paid-pack assets, exact curves, exact meshes, exact textures, or hidden values.
- Values from sources must be labeled `observed`, `inferred`, or `tuned` by the owning process doc or effect packet.
- Before implementation, classify the target, identify the owning internal process doc or concrete external reference process, and break down the load-bearing artifacts and mechanisms enough to implement and verify them.
- If the source process is unavailable, insufficiently broken down, or not understood well enough to say what artifact to create, what mechanism drives it, where it lives, and how to verify it, stop and get explicit user approval before exploration, prototyping, or substituting another method.

## PPF Check

Before implementation on a process-governed task, provide:

```text
PPF CHECK
Objective:
Proven process:
My planned implementation:
Same method class: YES/NO
If NO, why:
User approval required before proceeding: YES/NO
Verification evidence:
```

If `Same method class` is `NO`, stop and get explicit user approval before proceeding.

## Artifact Parity Gate

For process-governed visual, media, import, animation, audio, or VFX work, include:

```text
ARTIFACT PARITY GATE
Reference artifact/category:
Role: Primary/Secondary
Required: YES/NO
Planned artifact/path:
Status: SAME/EQUIVALENT/MISSING/DEFERRED
Evidence:
```

- Dropping, deferring, or replacing a required primary artifact means `Same method class` is `NO` unless the user explicitly approves the substitution.
- A required primary artifact marked `EQUIVALENT` or `DEFERRED` requires explicit user approval before implementation.
- Generic seed/support effects cannot satisfy a required primary visual carrier.
- For Niagara VFX, the slash, trail, projectile, aura, or attack silhouette must be produced inside the Niagara asset, renderer material, renderer mesh/ribbon, or emitter logic. Actor-side component transforms may place the system in the world but may not be the source of the silhouette.

## Mechanism Manifest

For process-governed visual, media, animation, audio, generated-content, and VFX tasks, add a mechanism manifest after the artifact parity gate and before implementation.

Mechanisms are verbs or behaviors, not nouns: motion, sampling, timing, panning, erosion, dissipation, blending, masking, import/reload, capture, validation. A mechanism is required when removing it changes the identity of the result rather than only reducing polish.

Required mechanisms must come from the owning process doc, effect packet, or reference/source breakdown. Do not infer missing load-bearing mechanisms from vague target wording. If a mechanism is not understood well enough to implement and verify, mark it `MISSING` and stop before implementation.

```text
MECHANISM MANIFEST
Reference/source:
Required mechanisms:
  1. Mechanism:
     Required: YES/NO
     Planned implementation:
     Evidence needed:
  2. Mechanism:
     Required: YES/NO
     Planned implementation:
     Evidence needed:
```

Same toolchain plus matching artifacts cannot pass if required mechanisms are missing. If any required mechanism is `MISSING`, `DEFERRED`, or replaced by a different behavior class, report `Same method class: NO` or `PARTIAL` and get explicit user approval before proceeding or accepting the result.

## Anti-Lookalike Rule

Name the cheapest wrong result that could pass a weak gate, then name and prove the discriminator that separates the intended result from that lookalike.

Examples:

- A static crescent mask is not a slash; a slash needs a moving, time-layered carrier with reveal and dissipation.
- A still image is not video proof; temporal behavior needs frame-range evidence.

## Closeout

At completion on a process-governed task, provide:

```text
PPF CLOSE
Process used:
Matches declared process: YES/NO
Evidence:
```

For any task that required a mechanism manifest, also provide:

```text
MECHANISM CLOSE
Mechanism:
Status: PRESENT/ABSENT/DEFERRED
Evidence:
Discriminator test:
Reported status: FULL/PARTIAL
```

`FULL` requires every required mechanism to be `PRESENT` with evidence. Any `ABSENT` or `DEFERRED` required mechanism means `PARTIAL`; do not describe a partial result as complete. Temporal mechanisms such as motion, animation, timing, reveal, erosion, and dissipation require multi-frame evidence, not a single still.
