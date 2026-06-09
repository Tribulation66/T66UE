# Original Prompt - Fall Guys Physics Ownership Stage 1

User approved reopening the hero physics architecture away from the old pure-Chaos/PAC-off direction.

Requested work:

- Implement Stage 1 physics ownership first.
- Create the dedicated broad `Physics` ownership layer, not an obstacle-only layer.
- Organize docs/models/process ownership for future work.
- Clean up stale direction from previous pure-Chaos/PAC-off work where safe.
- Do not start the full hero active-ragdoll runtime MVP yet.
- Next answer should include a concrete implementation plan for Stage 2 hero active-ragdoll MVP.
- Stage 2 plan must use Hero 1 Chad as the MVP.
- Stage 2 must include the model, rigging, animation, PhysicsAsset, and hero physics component work together because rigging and physics are coupled.
- User expects the eventual movement model to replace roll with a forward leap/jump and simplify movement around Fall Guys-like locomotion.

Working task:
Operator: Codex
Validator: Claude
Scope: Implement Stage 1 physics ownership only: create/update repo documentation and ownership routing for the reopened broad Physics architecture, update stale pure-Chaos/PAC-off policy notes, and produce a concrete Stage 2 hero active-ragdoll MVP plan for Hero 1 Chad including rigging/animation. No broad source deletion, model deletion, or Blender/Unreal asset mutation in this pass.
Stop condition: Stage 1 docs/routing are written, Stage 2 plan is written, Claude cross-check is incorporated, and verification is reported.

Key constraints:

- No native goal tools.
- Respect T66 root `AGENTS.md` and `OPERATOR_VALIDATOR_PROTOCOL.md`.
- `.t66/operator-state.json` selects Codex Operator and Claude Validator.
- Planning-only boundaries still apply to Stage 2; Stage 2 implementation will be a future pass.
- Broad source/asset deletion is out of scope for this first ownership pass. Cleanup here means stale policy/docs and explicit archive/indexing.
- Runtime gameplay changes are out of scope, so no build/staged verification is required unless a source change accidentally occurs.

Live repo facts:

- `Gameplay/README.md` has no `Physics` area.
- `Gameplay/GAMEPLAY_AGENTS.md` currently lists gameplay runtime systems but no physics trigger/owner.
- `Source/T66/Gameplay/pending_issues_Gameplay.md` still records the old "pure Chaos ragdoll is the hero direction" resolved issue and says PAC should not be tuned unless architecture is reopened. The architecture is now reopened by user approval.
- `Model Generation/Rigging and Animation/RIGGING_ANIMATION_AGENTS.md` says broad automated hero/humanoid rigging research is out of scope for that folder.
- `Model Generation/Instructions/13_FRIENDSLOP_RAW_HUMANOID_RIGGING_INSTRUCTIONS.md` is the canonical FriendSlop raw humanoid rigging process for skeletal FBX / later PhysicsAsset / PAC work.
- `Content/Data/CharacterVisuals.csv` currently has:
  - `Hero_1_Chad` using `/Game/Characters/Heroes/Hero_1/Chad/FriendSlopRaw/Skeletal/SK_Hero_1_Chad_Male_FriendSlop`.
  - `Hero_1_Chad_DemoSkin` using static mesh `/Game/Characters/Heroes/Hero_1/Chad/FriendSlopRaw/SM_Hero_1_Chad_Male`.
- Current raw source GLB exists at `Model Generation/Runs/Pixal3D/FriendSlopProbe_Hero1Male_20260604_1415/Outputs/Hero_1_Chad_Male.glb`.
- Existing FriendSlop physics asset report shows a TestRoom asset with 18 bodies and 17 constraints, but the new always-on active-ragdoll direction needs a stricter hero physics-ready rig/asset contract.

Please give an independent read-only answer for Stage 1 ownership and Stage 2 MVP planning, then cross-review Codex's draft after edits.
