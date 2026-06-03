Working task:
Operator: Claude.
Validator: Codex.
Scope: Implement Hero 1 DOT weapon placeholder structure in C:\UE\T66. Mini/minigame systems are out of scope.
Stop condition: Code/docs/proof artifacts are updated, focused compile and Unreal-owned gameplay video proof are attempted, and a complete Operator packet is written for Codex validation.

User-approved design:
- DOT weapon should use a single projectile shot.
- Reuse the existing Bounce moving-projectile infrastructure where practical.
- Once the projectile hits the enemy, DOT ticking damage should occur.
- Three small sphere projectiles/markers should spawn on the enemy as placeholder visuals.
- Important: the user's approved default is one authoritative DOT payload using existing DOT tuning. The three spheres are visual applicators/markers and must not triple DOT damage.

Read and follow:
- AGENTS.md
- Gameplay/GAMEPLAY_AGENTS.md
- Gameplay/Combat/VFX_PROCESS_INDEX.md
- Gameplay/Combat/CombatVFXAuthoringProcedure.md
- Gameplay/Combat/CombatVFXDefinitionOfDone.md
- Gameplay/Combat/CombatVFXVisualDamageAlignmentContract.md
- Gameplay/Combat/CombatVFXImpactContextContract.md
- Gameplay/Combat/Hero1AxeDOTMechanismPacket.md
- Gameplay/Combat/CombatVFXGeneratedAssetPolicy.md
- Gameplay/Combat/MASTER_COMBAT.md
- pending issues in touched folders

PPF CHECK:
Objective: Hero 1 DOT reads as a single moving weapon shot followed by target-attached DOT applicator markers and data-authoritative ticking damage.
Proven process: Combat VFX process plus the established Bounce moving-projectile seam and existing DOT combat lane.
My planned implementation: Use the current visual-only projectile mover/callback pattern for one hero-to-target DOT shot; on arrival apply/stage or reveal the existing DOT lane and spawn three small target-following sphere markers for the DOT duration; keep damage authority in RunState DOT and combat component code.
Same method class: YES.
If NO, why: N/A.
User approval required before proceeding: NO, because the user approved this damage semantic.
Verification evidence: compile, logs, video, and packet close.

ARTIFACT PARITY GATE:
Reference artifact/category: Hero 1 DOT placeholder weapon projectile and persistent target applicator markers.
Role: Primary for this temporary structure proof, deferred for final visual art.
Required: YES for placeholder proof; final Niagara aura art is DEFERRED.
Planned artifact/path: Runtime temporary projectile/mesh visuals or existing reusable placeholder components; no new final production Niagara asset unless already safely reusable.
Status: EQUIVALENT for placeholder; DEFERRED for final DOT Niagara art.
Evidence: gameplay video and logs proving movement, impact, marker attachment, and DOT ticks.

MECHANISM MANIFEST:
Reference/source: Bounce moving projectile path and existing RunState DOT application.
Required mechanisms:
  1. Mechanism: Single moving hero-to-target shot
     Required: YES
     Planned implementation: Reuse or generalize the Bounce visual-only mover/callback for a one-link DOT path.
     Evidence needed: video frame range and log showing one initial projectile.
  2. Mechanism: Hit-triggered marker reveal/spawn
     Required: YES
     Planned implementation: spawn/reveal exactly three small sphere markers on projectile arrival/impact around the target.
     Evidence needed: log count and video showing markers at target, not hero.
  3. Mechanism: Data-authoritative DOT ticking
     Required: YES
     Planned implementation: keep existing RunState ApplyDOT lane and single source semantics; do not multiply by marker count.
     Evidence needed: DOT application/tick logs and damage numbers.
  4. Mechanism: Target-following persistence
     Required: YES
     Planned implementation: attach to or update with target while DOT duration is active.
     Evidence needed: frame range over DOT duration.
  5. Mechanism: Impact context publication
     Required: YES
     Planned implementation: keep/preserve weapon DOT impact context at official target aim/impact point; log enough for context proof.
     Evidence needed: impact-context log fields.

Implementation guidance:
- Start from Source/T66/Gameplay/T66CombatComponent.cpp around PerformDOT and the Bounce helpers StageBounceProjectileChain / SpawnBounceLinkProjectile.
- Source/T66/Core/RunState/T66RunStateSubsystem_Idols.cpp stores DOT instances by target plus SourceIdolID, so do not create three independent source IDs for this task.
- If you need a helper, prefer a reusable one-link projectile staging helper over copy-pasting Bounce-only logic.
- Preserve Bounce behavior and existing AOE/Pierce behavior.
- If a final DOT Niagara binding does not exist, use a clear temporary blue sphere marker path and document it as temporary.
- Add/extend automation proof mode if needed. Use the standard Unreal-owned capture script/process.
- Update Hero1AxeDOTMechanismPacket.md from infrastructure-only to reflect this temporary structure proof, without claiming final visual fidelity or production Niagara approval.
- Update MASTER_COMBAT.md only if runtime combat behavior changed materially.

Compile/proof:
- Run a focused build/compile appropriate for T66 after edits.
- Run an Unreal-owned gameplay video capture using Scripts/CaptureT66GameplayVideo.ps1, with standard VFX proof camera and evidence bundle if practical.
- Ensure capture disables camera wall-occlusion contamination per the current process.

Operator packet requirement:
Write a completion packet under Reports/AgentReviews/Hero1DOTWeapon/claude_completion_packet.md containing:
- First line: `Operator Packet: COMPLETE` if and only if implementation and attempted verification are complete; otherwise `Operator Packet: INCOMPLETE`.
- Summary of changes.
- Files touched.
- PPF close.
- Mechanism close.
- Visual/damage alignment close.
- Impact context close.
- Exact verification commands and results.
- Video/log/evidence paths.
- Any skipped verification and why.
- Token usage if exposed.
