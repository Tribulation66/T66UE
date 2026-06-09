Result: OK

## Independent Answer

There is no single magic parameter that prevents ragdoll flattening — and any answer that names one is guessing. Flattening is a structural failure of the PhysicsAsset/rig, and pose drive cannot paper over it. The honest split:

**Guaranteed mechanical requirements (must be true at the PhysicsAsset/rig level — non-negotiable):**

1. **Collision bodies must have real volume matching each limb.** Flattening is overwhelmingly caused by collision primitives that are too thin, degenerate, or default-sized, so the solver lets them interpenetrate the floor and each other into a pancake. Capsules/spheres on pelvis, spine, thighs, calves, upper/lower arms must be authored to fill the mesh — not auto-fit slivers. The Stage 2 seed reports 18 bodies and `min_bone_size 4.0`, but does **not** prove the shape radii are authored to limb volume. This is the most likely root cause and must be verified first.

2. **Joint angular limits must actually constrain (swing1/swing2/twist Limited, not Free).** A flat silhouette means spine/neck/limb constraints are folding past anatomical range. "Joint limits block inversion" is already in your acceptance criteria — it must hold per-constraint in the PhysicsAsset, not just on the runtime hip anchor. The hip anchor in `T66HeroPhysicsComponent.cpp:468-473` only limits the pelvis-to-capsule relationship; it does nothing for inter-body folding.

3. **Solver iteration counts high enough to resist the impact.** Under large impulses (your `MaxReactionVelocityChange 3600`, `KnockdownSpeedThreshold 1700`), low Chaos position/velocity iteration counts let constraints stretch and bodies overlap, which reads as collapse + then the divergence guard fires repeated resyncs (`ResyncDetachedBodiesToActor`). Raise solver iterations on the bodies.

4. **Intentional self-collision policy.** Disable collision between directly-jointed neighbors (avoids jitter) but keep it enabled between non-adjacent bodies so the torso/limbs can't pass through each other into a flat stack.

5. **Pelvis-weighted mass distribution** (already an acceptance item) so the body has a stable center rather than limbs dominating.

**Tuneable values (refine only after 1–5 are correct):** `PoseOrientationStrength`, `BalancedPoseStrengthMultiplier`, anchor linear/angular strengths, recovery timing. These shape *recovery feel*; they cannot fix a structurally collapsing asset. Cranking pose drive against a flattened body is exactly what produces the spiky silhouette + repeated resyncs you're seeing.

**What must be fixed next:** open the Hero 1 PhysicsFirst PhysicsAsset and fix body shape volume and per-constraint angular limits, then raise solver iterations — in that order — before touching any drive multipliers. This is precisely the dedicated PhysicsAsset tuning loop that `pending_issues_Physics.md` already scopes out of Stage 3 runtime; that scoping is correct.

## Evidence Checked
- `pending_issues_Physics.md` — flattening/spiky silhouette + repeated resyncs, explicitly scoped to a PhysicsAsset/rig tuning loop.
- `PhysicsAssetPipeline.md` — acceptance criteria already require volume bodies, inversion-blocking joint limits, pelvis-weighted mass, reviewed CCD/sleep.
- `physics_first_hero1_physics_asset_report.json` — 18 bodies / 17 constraints / `min_bone_size 4.0`; continuous pelvis-rooted constraint graph. Does not record shape radii, angular limits, self-collision, or solver iterations.
- `T66HeroPhysicsComponent.cpp/.h` — runtime already does active sim below pelvis, local-space physical animation drive, hip anchor constraint, CCD on, divergence guard. Confirms runtime is not the gap; the asset is.

## Questions Or Blockers
None that require the user. The models can answer this directly. The only true verification step (inspecting body shape sizes / per-constraint limits) is a PhysicsAsset-editor task for the next implementation pass, not a user decision.

## Caveats
- The constraint graph being "sane" in the seed report does **not** confirm limits are *Limited* or shapes have volume — those fields aren't in the JSON, so don't treat the seed as evidence they're correct.
- Codex's draft should avoid implying a runtime/profile tweak will fix flattening; the fix is in the asset. If the draft leads with drive-strength tuning, reorder it behind the structural fixes.
- I could not confirm the actual Chaos solver iteration settings or per-constraint limit values without opening the asset, so item 2 and 3 are stated as requirements to verify, not as confirmed defects.
