# Current Handoff Prompt

Use this prompt in a fresh Codex session to continue from the current state without losing context:

```text
Use [MASTER_WORKFLOW.md](C:/UE/T66/Model Generation/MASTER_WORKFLOW.md) as the authoritative source of truth for this task. Also read [RUN_HISTORY.md](C:/UE/T66/Model Generation/RUN_HISTORY.md), [NEXT_STEPS.md](C:/UE/T66/Model Generation/NEXT_STEPS.md), [KNOWN_ISSUES.md](C:/UE/T66/Model Generation/KNOWN_ISSUES.md), and [ENVIRONMENT_LOCK.md](C:/UE/T66/Model Generation/ENVIRONMENT_LOCK.md) before doing work.

Important current state:
- The JSX-parity TRELLIS.2 environment has already been restored and documented.
- The best accepted Arthur raw body is [Arthur_HeroReference_Full_White_S1337_D80000_Trellis2.glb](C:/UE/T66/Model Generation/Runs/Arthur/Raw/Arthur_HeroReference_Full_White_S1337_D80000_Trellis2.glb).
- The best accepted Arthur low-poly body is [Arthur_HeroReference_Full_White_S1337_D80000_Decimate40k.glb](C:/UE/T66/Model Generation/Runs/Arthur/Raw/Arthur_HeroReference_Full_White_S1337_D80000_Decimate40k.glb).
- The accepted raw sword source is [Arthur_ExcaliburProxy_FlatGreen_Tight_S1337_Trellis2.glb](C:/UE/T66/Model Generation/Runs/Arthur/Raw/Arthur_ExcaliburProxy_FlatGreen_Tight_S1337_Trellis2.glb).
- The current sword-hold attempt [Arthur_HeroReference_Full_White_S1337_D80000_Decimate40k_WithSword_ValidatedHold.glb](C:/UE/T66/Model Generation/Runs/Arthur/Raw/Arthur_HeroReference_Full_White_S1337_D80000_Decimate40k_WithSword_ValidatedHold.glb) is NOT accepted. Treat it as a failed-but-useful reference only.
- The working Blender scene is [Arthur_EasyEnemy_Lineup.blend](C:/UE/T66/Model Generation/Scenes/Arthur_EasyEnemy_Lineup.blend).
- The easy enemy batch already exists and should be used later for rigging / baked-mesh exploration, not regenerated from scratch yet.

Primary goal:
- Get the sword to look like it is actually being held by Arthur.

Sword-hold rules:
- Start in Blender with the existing Arthur low-poly body plus separate sword asset.
- Try transform-only sword placement first.
- Save at least these screenshots for each serious attempt:
  - front
  - side
  - oblique
  - one user-like gameplay or showcase angle
- For each rejected attempt, state which angle failed and why before making the next adjustment.
- Reject any attempt where the grip does not clearly overlap the hand volume in any one of those views.
- Do not call the sword solved until the user agrees it reads correctly.
- If two or three serious transform-only attempts keep failing for the same reason, escalate to a minimal hand-pose edit, socket rule, or lightweight rig step instead of endlessly nudging transforms.

Secondary goal:
- After the sword hold is accepted, explore rigging and baked-mesh workflows for Arthur and the easy enemies.

Rigging / bake exploration rules:
- Start with Arthur first.
- Then test one easy enemy family as a representative case before generalizing.
- Determine whether the best path is:
  - baked static meshes only
  - rigged characters
  - or both outputs from the same source assets
- Record the pros, blockers, and recommended pipeline in the docs.

Process rules:
- If Blender MCP is disconnected, reopen Blender and rerun [start_blender_mcp.py](C:/UE/T66/Tools/Trellis2/start_blender_mcp.py) before falling back.
- Save PNG review images to disk even if chat previews fail.
- Update [RUN_HISTORY.md](C:/UE/T66/Model Generation/RUN_HISTORY.md), [MASTER_WORKFLOW.md](C:/UE/T66/Model Generation/MASTER_WORKFLOW.md), [KNOWN_ISSUES.md](C:/UE/T66/Model Generation/KNOWN_ISSUES.md), and [NEXT_STEPS.md](C:/UE/T66/Model Generation/NEXT_STEPS.md) with any real findings.
- Do not restart hero TRELLIS seed sweeps unless you find a concrete reason the current accepted body can no longer support the pipeline.
```
