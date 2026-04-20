# Next Steps

This is the current recommended work order as of `2026-04-19`.

## Priority Order

1. Fix Arthur's sword hold so it clearly reads as truly held by the hero.
2. If transform-only placement is not enough, test the smallest possible hand-pose or lightweight rig adjustment.
3. Decide the rigging direction for the hero and easy enemy batch.
4. Explore baked-mesh and baked-texture workflow for the hero and easy enemies.
5. Only after the local character pipeline is stable, continue to the next enemy families or Unreal import.

## Immediate Experiment Plan

For the next local Blender-focused pass:

- start from:
  - [Arthur_EasyEnemy_Lineup.blend](C:/UE/T66/Model%20Generation/Scenes/Arthur_EasyEnemy_Lineup.blend)
  - [Arthur_HeroReference_Full_White_S1337_D80000_Decimate40k.glb](C:/UE/T66/Model%20Generation/Runs/Arthur/Raw/Arthur_HeroReference_Full_White_S1337_D80000_Decimate40k.glb)
  - [Arthur_ExcaliburProxy_FlatGreen_Tight_S1337_Trellis2.glb](C:/UE/T66/Model%20Generation/Runs/Arthur/Raw/Arthur_ExcaliburProxy_FlatGreen_Tight_S1337_Trellis2.glb)
- treat [Arthur_HeroReference_Full_White_S1337_D80000_Decimate40k_WithSword_ValidatedHold.glb](C:/UE/T66/Model%20Generation/Runs/Arthur/Raw/Arthur_HeroReference_Full_White_S1337_D80000_Decimate40k_WithSword_ValidatedHold.glb) as a failed-but-useful reference, not a solved result
- try transform-only sword placement first
- save at least these review PNGs for each serious sword attempt:
  - front
  - side
  - oblique
  - one user-like gameplay or showcase angle
- explicitly note which angle failed and why before trying the next adjustment
- reject the pose immediately if any view shows:
  - grip outside the hand volume
  - sword reading as placed beside the hand
  - blade direction in the wrong family
- only call the sword solved after explicit user approval
- if two or three serious transform-only attempts still fail for the same reason, escalate to hand-pose or rig work instead of repeating the same transform loop

## After Sword Hold Is Solved

Run these in order:

1. Test whether a minimal hand edit or simple rig is needed for the hero.
2. Explore whether Arthur should remain a baked static mesh, become a rigged character, or support both outputs.
3. Run the same rigging / bake exploration on one easy enemy first, then generalize if it works.
4. Record the chosen direction in [MASTER_WORKFLOW.md](C:/UE/T66/Model%20Generation/MASTER_WORKFLOW.md) and [RUN_HISTORY.md](C:/UE/T66/Model%20Generation/RUN_HISTORY.md).

## Current Proven Settings

- Hero raw generation:
  - input: [Arthur_HeroReference_Full_White.png](C:/UE/T66/Model%20Generation/Runs/Arthur/Inputs/Arthur_HeroReference_Full_White.png)
  - seed: `1337`
  - `X-Texture-Size: 2048`
  - `X-Decimation: 80000`
- Hero low-poly follow-up:
  - preferred: `40k` tris
  - alternate: `20k` tris with slight softening
- Easy enemy raw generation:
  - seed: `1337`
  - `X-Texture-Size: 2048`
  - `X-Decimation: 200000`
- Easy enemy retopo targets:
  - bipeds: `24k`
  - slime: `10k`
  - flying: `14k`

## Stop Conditions

Stop and reassess if:

- the next agent starts treating the sword as already solved
- the environment drifts away from [ENVIRONMENT_LOCK.md](C:/UE/T66/Model%20Generation/ENVIRONMENT_LOCK.md)
- a new sword pass is approved from only one or two screenshots
- repeated transform-only sword attempts keep failing and nobody escalates to a hand-pose or rig step

## Defer Until Later

- new split head/body hero generation
- new full hero TRELLIS seed sweeps
- Unreal import
- next difficulty-family batch beyond the existing easy enemies
