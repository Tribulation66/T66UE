# Decision Block — Hero 1 Bounce Visible Carrier Fix

## Status

`NEEDS_HUMAN_DECISION`

Claude and Codex agree that the runtime Bounce structure is correct but the visual proof still does not satisfy Pablo's correction.

## Current Confirmed State

- Bounce fires exactly two links in the proof: `LinkIndex=0`, then `LinkIndex=1`, no `LinkIndex=2`.
- Damage is preserved: primary `28`, second `26`.
- The authored Niagara carrier remains `/Game/VFX/Hero1/Axe/Bounce/NS_Hero1AxeBounce_MeshSlash`.
- The current capture still does **not** prove a readable slash travelling from hero to primary and then primary to second.
- Newest current proof path: `C:\UE\T66\Saved\VideoCaptures\Hero1BounceVisibleCarrierFix_20260529g\hero1axebouncevfxbinding.mp4`
- Current Operator report: `C:\UE\T66\Reports\AgentReviews\Hero1BounceVisibleCarrierFix\claude_operator_report.md`

## Why This Is Blocked

Two runtime-only approaches were tested:

1. Spawn the authored Niagara slash at world location and drive its world position along the visual mover.
2. Spawn the authored Niagara slash attached to the visual mover and let it inherit mover travel.

Both preserved the method class, compiled, and produced two-link logs, but the visible slash still presents near the hero/owner rather than clearly originating at the link start and travelling through the segment.

Continuing without approval would either modify/regenerate the Niagara asset or use a runtime visual trick that may violate the approved VFX method class or the user's "don't overcomplicate" direction.

## Decision Options

### Option A — Re-author the Bounce Niagara asset

Approve a focused asset/commandlet pass to re-author `NS_Hero1AxeBounce_MeshSlash` so the slash itself reads as a travelling streak over the segment.

Pros:
- Best fit for the combat VFX process.
- Keeps Niagara as the primary carrier.
- Most likely to satisfy the user's visual expectation robustly.

Tradeoff:
- Crosses the asset-regeneration gate and may touch binary `.uasset` outputs.

### Option B — Investigate possible owner-anchored masking

Approve another diagnostic pass to determine whether an owner-anchored effect is masking or confusing the real carrier.

Pros:
- No immediate asset rewrite.
- Could reveal a smaller code/config issue.

Tradeoff:
- Uncertain payoff; may still end in Option A.

### Option C — Runtime multi-stamp progression

Approve spawning the existing authored slash at staggered points along each segment so the eye reads progression from hero to target.

Pros:
- No asset rewrite.
- Uses the first-iteration visual directly.

Tradeoff:
- More of a runtime illusion than a moving carrier, and may conflict with "don't overcomplicate."

### Option D — Accept current behavior

Accept the current state as structurally correct but visually partial.

Pros:
- No further work.

Tradeoff:
- Does not satisfy the stated visual expectation.

## Recommendation

Option A is recommended if the goal is a clean, reusable weapon-projectile process for future weapons and idols.
