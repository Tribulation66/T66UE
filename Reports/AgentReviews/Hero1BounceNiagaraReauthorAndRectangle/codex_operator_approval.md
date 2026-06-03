Codex Approval: APPROVE

Task: Implement the user-approved Option A for Hero 1 Bounce: re-author/regenerate the authored Bounce Niagara carrier so it visibly travels from hero to primary and then primary to second, and investigate the recurring cream-colored rectangle in the proof view.

Approved Operator: Claude (`claude-opus-4-8`)

Approved scope:
- Focused Bounce VFX asset/commandlet/code/docs changes needed to make `/Game/VFX/Hero1/Axe/Bounce/NS_Hero1AxeBounce_MeshSlash` read as a travelling slash projectile.
- Expected files may include:
  - `Source/T66/Gameplay/T66Hero1AxeBounceVFXCommandlet.cpp`
  - `Source/T66/Gameplay/T66CombatComponent.cpp`
  - `Source/T66/Gameplay/T66HeroProjectile.cpp`
  - `Source/T66/Gameplay/T66HeroProjectile.h`
  - `Source/T66/Gameplay/T66PlayerController_Overlays.cpp`
  - `Content/VFX/Hero1/Axe/Bounce/*.uasset`
  - `Content/Data/CombatVFXBindings.csv` and `Content/Data/DT_CombatVFXBindings.uasset` only if a binding change is truly required
  - Bounce-specific docs under `Gameplay/Combat/`
  - proof/report files under `Reports/AgentReviews/Hero1BounceNiagaraReauthorAndRectangle/`
- Investigate the cream-colored rectangle/block seen in prior proof videos. Trace it to its owning source actor/component/material/capture path. If it is erroneous proof-harness clutter, remove/hide it in the proof path. If it is intentional gameplay geometry, document what it is and why it appears.
- Compile and capture verification through repo-owned scripts/commandlets.

User-approved decision:
- Pablo approved Option A from `Reports/AgentReviews/Hero1BounceVisibleCarrierFix/decision_block.md`.
- This approval permits a focused asset/commandlet pass and binary `.uasset` outputs for the Bounce Niagara carrier, as long as the changes are scoped and documented.

Required preservation:
- Preserve Bounce damage authority in combat logic.
- Preserve target selection and chain damage behavior.
- Preserve exactly one visible link in flight per segment.
- Preserve link 0 hero-to-primary, then link 1 primary-to-second sequencing.
- Preserve official per-link impact-context publication.
- Preserve Niagara/authored-asset method class: the accepted primary silhouette must live in the Niagara asset/material/mesh/ribbon/emitter logic, not in procedural debug geometry or a non-Niagara runtime illusion.
- Preserve Mini/minigame exclusion.

Required visual result:
- Link 0: visible authored Bounce slash starts near the hero, has at least one readable mid-path frame, and reaches/aligns with the primary target.
- Link 1: visible authored Bounce slash starts near the primary target, has at least one readable mid-path frame, and reaches/aligns with the second target.
- The carrier should resemble the first readable Bounce iteration, but must move from the correct origins instead of appearing on enemies.
- The proof view must be clean: no cream/yellow block or unrelated gray/yellow object occluding or distracting from the hero/enemies.

Required verification:
- Commandlet/import/generation logs for any regenerated Bounce assets.
- Focused C++ compile for `T66Editor Win64 Development`.
- Runtime log evidence for `CombatVFXBounceLinkProjectile` showing `LinkIndex=0`, `LinkCount=2`, followed later by `LinkIndex=1`, `LinkCount=2`, and no `LinkIndex=2`.
- Runtime damage evidence proving Bounce damage still lands on intended targets.
- Unreal-owned MP4 in the standard original proof camera view.
- Contact sheet or selected frame evidence that labels:
  - link 0 start near hero
  - link 0 mid-path
  - link 0 at/near primary
  - link 1 start near primary
  - link 1 mid-path
  - link 1 at/near second
- Rectangle investigation evidence: source anchor, log/capture proof, and whether it was removed or documented.

Excluded:
- Git commits, pushes, tags, resets, cleans, or broad Git/LFS scans.
- Unrelated weapons, idols beyond incidental preservation, DOT/AOE/Pierce redesign, balance/stat tuning, Mini/minigame systems.
- Reverting user or peer changes.
