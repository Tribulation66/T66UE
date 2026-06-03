Codex Approval: APPROVE

Task: Implement Hero 1 DOT weapon placeholder structure using the user's approved default damage semantics.

Approved scope:
- Runtime code needed for Hero 1 DOT weapon behavior:
  - one visible moving hero-to-target projectile before DOT application,
  - one existing data-authoritative DOT lane using current weapon/hero DOT tuning,
  - three small target-attached or target-following visual sphere markers for the DOT duration,
  - logs/proof hooks that show projectile spawn/arrival, DOT application, marker spawn, and DOT tick damage.
- Automation/capture code needed to prove the DOT path through the existing Unreal-owned gameplay video process.
- Process docs and the Hero 1 DOT packet needed to record the temporary placeholder status, visual/damage alignment, impact-context policy, and proof evidence.
- Focused compile and Unreal-owned gameplay video proof if physically possible.

Required constraints:
- Do not alter DOT balance unless required to preserve the existing single DOT payload.
- Do not make each sphere an independent damage lane. The spheres are visual applicators for the existing single DOT payload.
- Do not touch Mini/minigame systems.
- Do not make final Niagara art or imagegen assets for DOT in this phase.
- Do not replace combat-authoritative damage with visual, mesh, Niagara, or projectile-collision authority.
- Preserve existing Bounce behavior.
- Do not revert unrelated user or prior-agent changes.

Expected evidence:
- Source diff summary.
- Focused compile command/output or explicit blocker.
- Standard Unreal-owned DOT gameplay video path and logs or explicit blocker.
- Evidence that the capture shows one initial projectile and three sphere markers after impact.
- Evidence that DOT tick damage occurs through the existing DOT damage source rather than three multiplied independent lanes.
