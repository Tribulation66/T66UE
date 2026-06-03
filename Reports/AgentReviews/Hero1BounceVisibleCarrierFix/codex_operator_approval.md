Codex Approval: APPROVE

Task: Fix the Hero 1 Bounce weapon VFX presentation so the authored Bounce horizontal slash carrier is visibly fired from the hero to the primary target, then visibly travels from that target to a second target, in the standard original-camera proof view.

Approved Operator: Claude (`claude-opus-4-8`)

Approved scope:
- Read and edit only the files required for the Bounce visual carrier presentation and proof harness.
- Expected runtime/code scope may include:
  - `Source/T66/Gameplay/T66CombatComponent.cpp`
  - `Source/T66/Gameplay/T66CombatComponent.h`
  - `Source/T66/Gameplay/T66HeroProjectile.cpp`
  - `Source/T66/Gameplay/T66HeroProjectile.h`
  - `Source/T66/Gameplay/T66PlayerController_Overlays.cpp`
  - Bounce-specific docs under `Gameplay/Combat/`
  - this task report folder under `Reports/AgentReviews/Hero1BounceVisibleCarrierFix/`
- Compile and capture verification through existing repo scripts.
- Produce Unreal-owned video proof in the same standard camera family used by the accepted original Bounce proof, with enemies visible and no yellow proof block in front of the hero.

Required preservation:
- Preserve Bounce damage authority in combat logic.
- Preserve target selection and chain damage behavior.
- Preserve exactly one visible link in flight per segment.
- Preserve link 0 hero-to-primary, then link 1 primary-to-second sequencing.
- Preserve official per-link impact-context publication.
- Preserve the authored Bounce Niagara slash as the primary visual silhouette. Actor-side transforms may place/orient/scale the system, but the accepted carrier cannot become a procedural debug mesh, cube, or actor-arranged shape.
- Preserve Mini/minigame exclusion.

Current user correction:
- The previous standard-view video had correct camera/target isolation, but the projectile was not visible enough. It looked like a side/outside projectile and damage happened without a readable moving Bounce slash.
- The expected visual should resemble the first iteration of the Bounce projectile/slash, but with motion starting from the hero and traveling to enemies instead of appearing statically on enemies.
- Do not overcomplicate the behavior. If the current moving-carrier implementation can be simplified while keeping the same Niagara carrier method class, do so.

Decision gate:
- If the only viable way to get the first-iteration look is to replace the Niagara-authored primary carrier with actor-side debug geometry, a static mesh-only carrier, a non-Niagara silhouette, or an unrelated new asset process, stop and return `NEEDS_HUMAN_DECISION` instead of implementing that substitution.
- If a broad asset-regeneration pass or binary `.uasset` rewrite is required, stop and request explicit approval before doing it unless the change is limited, clearly necessary, and documented in the Operator packet.

Required verification:
- Focused C++ compile for `T66Editor Win64 Development`.
- Runtime log evidence for `CombatVFXBounceLinkProjectile` showing `LinkIndex=0`, `LinkCount=2`, followed later by `LinkIndex=1`, `LinkCount=2`, and no third link.
- Runtime damage evidence proving Bounce damage still lands on the intended targets.
- Unreal-owned MP4 in the standard original proof camera view.
- Contact sheet or selected frame evidence showing the carrier visible over time on both travel segments.

Excluded:
- Git commits, pushes, tags, resets, cleans, or broad Git/LFS scans.
- Unrelated weapons, idols, DOT, AOE, Pierce redesign, balance/stat tuning, Mini/minigame systems.
- Reverting user or peer changes.
