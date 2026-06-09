Result: OK

## Independent Answer

This is a planning/review request that Codex and Claude can handle internally — no user decision blocks the rigging pass. The source GLB exists (`Outputs/Hero_1_Chad_Male.glb`, ~10.8 MB), the Rigging output folder is absent and must be created, and the FriendSlop raw-import guideline (doc 11) is present and authoritative.

**1. Key risks / must-have validation gates**

- **Outline/material scope (highest-value flag).** The opening line "import this model with the black outline" contradicts the detailed export spec ("no outline/stylization … skeleton + skinned mesh only") and doc 11 ("skip ToonStyle/Quad Retro/tint/outline unless explicitly approved"). Codex's conservative read (outline = reference/proof only, raw GLB material preserved for later Unreal rebind) is the correct default. Proceed that way and state it explicitly in the report; do not bake an outline into the exported FBX.
- **Bone count vs PhysicsAsset goal.** The whole point is to escape the 6-body/2-constraint failure. Gate: the exported hierarchy must literally contain `root → pelvis → spine_01 → spine_02 → spine_03 → neck_01 → head` plus the named arm/leg chains, with no zero/near-zero-length torso/limb bones. Count bodies-worth of bones before declaring success.
- **Transform bake.** Scale 1.0, applied rotation, feet on floor, ~180 cm preserved — verify after re-import, not just before export. Z-up, document forward axis (prefer UE +X). Do not fix yaw in Blender.
- **Weights.** ≤4 influences/vertex normalized, zero unweighted verts, no large root-only/pelvis-only blobs. This is the most common silent failure — make it a hard re-import gate.
- **Topology honesty.** If dense, keep-and-document; do not Decimate and call it production (doc 04).
- **Round-trip acceptance.** Re-import the exported FBX into Blender and confirm skeleton, mesh, weights, rest pose, and material slot all survive — this is the real acceptance gate, not the in-session state.

**2. What belongs in the central FriendSlop rigging process doc**

- Purpose/scope line: this is the canonical rig procedure for raw FriendSlop Pixal3D models; explicitly note the retired legacy Animated ToonStyle bridge is NOT used (per Rigging README).
- Source asset contract: which GLB, where runs live, output folder convention.
- Canonical bone hierarchy + naming convention (UE5-Mannequin-like) as a copy-paste reference.
- Transform-bake checklist (scale/rotation/floor/height) and the four-axis facing-proof procedure with how to record true visual front.
- Export settings block (Z-up, forward axis, skeleton+skinned only, material-slot preservation, no stylization).
- Weight QA thresholds and how to verify them.
- Re-import validation checklist (the acceptance gates above).
- "Things to avoid" list: duplicate roots, mesh-parented bones, leaf/end-bone junk, exporting control/helper bones as physics candidates, blind Decimate, solving yaw in Blender, baking outline/ToonStyle into the rig export.
- Unreal PhysicsAsset handoff notes: pelvis = central recovery body, expected body/constraint coverage, what "good enough structure" means.
- A short changelog/known-compromises section so future agents see what was deferred.

**3. Scope conflicts / blockers** — none that require the user. The only ambiguity (outline) has a safe documented default; proceed conservatively and surface it in the report.

## Evidence Checked

- Confirmed `Hero_1_Chad_Male.glb` exists in Outputs (~10.8 MB).
- Confirmed target Rigging folder does not yet exist (must be created — matches deliverable).
- Confirmed `11_FRIENDSLOP_RAW_PIXAL3D_IMPORT_GUIDELINES.md` and `04_BLENDER_PROCESSING_AND_RIGGING_INSTRUCTIONS.md` exist.
- Read the original prompt's scope, acceptance criteria, and repo/process context.

## Questions Or Blockers

- None requiring the user. Recommend Codex note the outline interpretation in the rig report so the user can object cheaply if they actually wanted the outline preserved — but the documented default (skip outline) is the correct path and does not block work.

## Caveats

- I did not open the GLB or inspect its existing skeleton/weights; whether it already has a usable armature vs. needs a fresh skeleton materially affects effort and is unverified here.
- Acceptance hinges on the re-import round-trip and actual bone/weight counts, which can only be confirmed after Codex runs the pass — treat the report's QA section as the real gate.
- "UE +X forward if practical" and final naming should defer to any existing clearly-better T66 convention if one is found during implementation.
