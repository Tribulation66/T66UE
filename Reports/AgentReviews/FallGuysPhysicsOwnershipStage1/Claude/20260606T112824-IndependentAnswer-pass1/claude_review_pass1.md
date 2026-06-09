Result: OK

## Independent Answer

This is a planning-and-docs ownership pass, internally resolvable by Codex+Claude. The architecture reopening is already user-approved, so no new user decision gates Stage 1. Verified live repo matches the prompt's stated facts, and `.t66/operator-state.json` confirms Codex Operator / Claude Validator.

**Stage 1 — broad Physics ownership (write these):**

1. **Create a dedicated broad `Physics` area, not obstacle-only.**
   - Add a `Physics` entry to `Gameplay/README.md` Areas list, described as the owner of hero/ragdoll/active-physics, knockback, PAC policy, obstacle/Chaos interaction — i.e. all physics-simulation ownership, not just obstacles.
   - Add physics trigger words (`physics, ragdoll, PAC, active ragdoll, knockback, Chaos, simulation`) and an owner line to `Gameplay/GAMEPLAY_AGENTS.md` (currently has none).
   - Create `Gameplay/Physics/PHYSICS_AGENTS.md` router that points to the physics-policy doc and to the FriendSlop rigging instruction doc 13 as the upstream rig/PhysicsAsset contract. Keep it a router, per AGENTS.md §3.
   - Add a `Gameplay/Physics/` policy/index doc recording the reopened broad-Physics direction and Stage 1/Stage 2 staging.

2. **Update stale pure-Chaos/PAC-off policy (amend, don't delete).** In `Source/T66/Gameplay/pending_issues_Gameplay.md`, the first Resolved issue says "PAC should not be tuned unless the hero physics architecture is explicitly reopened." Add a dated supersede note: architecture reopened 2026-06-06 by user approval; hero direction moves to broad active-physics ownership; the old pure-Chaos/PAC-off default is no longer the standing direction. Leave historical evidence references intact (it's a resolved-issue ledger, not live policy code).

3. **Keep deletions to docs/indexing only.** No source/asset/CSV/Blender/Unreal mutation, per scope. Cleanup = stale policy text + explicit archive/index pointers.

**Stage 2 — Hero 1 Chad active-ragdoll MVP plan (write the plan only):**
A concrete, coupled plan covering: (a) model — start from the live raw GLB `Model Generation/Runs/Pixal3D/FriendSlopProbe_Hero1Male_20260604_1415/Outputs/Hero_1_Chad_Male.glb`; (b) rigging — follow doc 13 canonical deform skeleton and the PhysicsAsset-readiness gate (real pelvis under root, 3 nonzero spine bones, distinct head/arm/leg chains, no near-zero/helper bones) so the asset clears the legacy "too few bodies/constraints" failure; (c) animation — minimal locomotion clips for the leap/jump-led movement model; (d) PhysicsAsset — stricter hero physics-ready contract than the existing 18-body/17-constraint TestRoom asset; (e) hero physics component — runtime ownership stub spec under the new Physics layer. Plan must explicitly stay planning-only and route rigging through doc 13, not the retired humanoid bakeoff.

## Evidence Checked
- `AGENTS.md` — no native goal tools, planning-only boundaries, PPF/process-fidelity, router discipline, commit/LFS rules.
- `Gameplay/README.md` and `Gameplay/GAMEPLAY_AGENTS.md` — confirmed no Physics area / no physics trigger owner.
- `Source/T66/Gameplay/pending_issues_Gameplay.md` — confirmed the resolved pure-Chaos/PAC-off issue and the "reopen" conditional.
- `Model Generation/Rigging and Animation/RIGGING_ANIMATION_AGENTS.md` — automated hero/humanoid rigging research is out of scope; user handles manual rigging; doc 13 bridge in scope.
- `Model Generation/Instructions/13_FRIENDSLOP_RAW_HUMANOID_RIGGING_INSTRUCTIONS.md` — canonical rig process, skeleton, PhysicsAsset-readiness gate.
- `.t66/operator-state.json` — Codex Operator, Claude Validator.

## Questions Or Blockers
None requiring the user. The reopening is already approved; everything else is docs/planning Codex can complete.

## Caveats
- **Folder-boundary tension Codex must word carefully:** `RIGGING_ANIMATION_AGENTS.md` declares automated hero/humanoid rigging research out of scope and says not to revive the retired bakeoff. The Stage 2 plan must frame rigging as the *manual* doc-13 process the user owns, not new automated rigging research — otherwise it contradicts that router.
- **PPF deferral:** Stage 2 is plan-only. The PPF check / artifact-parity gate and a `Gameplay/Physics/pending_issues_*.md` entry belong to the future Stage 2 implementation pass, not this one. The plan should name doc 13 as the proven process so the later PPF has an owner.
- **Movement model (roll→leap/jump) is the user's stated eventual intent, not an approved Stage 2 build scope.** The plan should present it as the locomotion target/direction, not commit to deleting roll now.
- **No build/verification expected** unless a source change accidentally occurs; report verification as "docs/plan only, no runtime change."
- Did not exhaustively read every subfolder router; if a physics-related doc already exists elsewhere, Codex should reuse/extend rather than duplicate the new Physics layer.
