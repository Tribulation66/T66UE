# Resume Prompt

The user answered the decision gate:

> Yeah, ignore any mid-change stuff. I want you to start from scratch. Do not restore anything, but also do not pick up where the mid-change stuff left off, because they might have done it with a bad foundation. You need to do everything from scratch.

# Active Task Contract

Working task:
Operator: Codex
Validator: Claude
Scope: Resume Stage 2 implementation after user decision: ignore the current mid-change asset state, do not restore old assets, and do not build on another agent's partial foundation. Build a fresh physics-first Hero 1 Chad Stage 2 path from the raw source model and update the standard/docs/code/data accordingly, while keeping Stage 3 active-ragdoll/PAC/hip-constraint work out of scope.
Stop condition: Fresh Stage 2 docs and implementation artifacts are produced as far as current tools allow, verification is attempted and reported, Claude cross-check is incorporated, and any hard blocker is documented once.

# Ground Rules

- Do not restore deleted old assets.
- Do not use another agent's existing mid-change content state as the foundation.
- Do use the raw source model:
  `Model Generation/Runs/Pixal3D/FriendSlopProbe_Hero1Male_20260604_1415/Outputs/Hero_1_Chad_Male.glb`
- Existing spike outputs under `.../Blender/Rigging` may be read as cautionary evidence only, not as the build foundation.
- Stage 3 active-ragdoll runtime component/PAC/hip-constraint work stays out of scope.
- Current tool paths found by Codex:
  - Blender: `C:\Program Files\Blender Foundation\Blender 5.1\blender.exe`
  - Unreal commandlet: `C:\Program Files\Epic Games\UE_5.7\Engine\Binaries\Win64\UnrealEditor-Cmd.exe`

# Requested Claude Output

Before Codex edits production files, provide a short read-only scope check:

1. Is proceeding from raw GLB despite the mid-change content state consistent with the user's decision?
2. What should Codex be careful not to reuse from the old spike rig/animation outputs?
3. What is the smallest implementation that can honestly count as "from scratch" if full production retopo/hand-weighting is too large for this pass?
4. What should final verification prove for this Stage 2 pass?
