# B.13 Mob HISM Rendering - Decision Block

## Review Artifact

- Review: `C:\UE\T66\Reports\AgentReviews\20260529_B13_MobHISM_VAT\20260529T041659-pass4\claude_review_pass4.md`
- Verdict: `NEEDS_HUMAN_DECISION`

## Current State

B.13 HISM/VAT rendering was attempted in the isolated tree `C:\UE\T66_B13_Worktree`.

The core technique works functionally:

- VAT frame selection via per-instance custom data works.
- Slime-only and all-family HISM runtime proofs passed.
- Spatial-cell material-offset positioning also rendered visible, co-located mobs.

The performance goal did not pass:

- Full-resolution pre-B.13 baseline: `189.65` median FPS, `156.16` 1% low, `72.03` 0.1% low.
- Every HISM/ISM candidate either regressed median FPS or crashed.
- Best final HISM probe: `179.36` median FPS, `157.71` 1% low, `139.87` 0.1% low. Lows improved, but median missed baseline by `10.30` FPS.

No B.13 renderer was copied back to live runtime source.

## Decision Needed

Choose the next B.13R direction, or confirm that B.13 should close as no-land for now.

Options:

1. **Partial instancing only**: keep per-mob mesh components for moving mobs, and instance only non-moving/death/pooled visual states.
2. **ISM pivot**: abandon HISM for basic mobs and build a reviewed ISM-only renderer, accepted only if it proves a full-resolution median win over `189.65`.
3. **Lower-frequency transforms / interpolation**: reduce visual transform update frequency and use interpolation/material offset. This is a gameplay-visual tradeoff and needs explicit approval.
4. **GPU/VAT crowd renderer**: move positions/frames into a manager-owned GPU buffer or texture instead of UE component instance transforms. Highest scope/risk, most data-oriented.
5. **Close B.13 as no-land**: keep the current per-mob renderer and move on to non-render cleanup/roster passes.

## Safe Default

Do not continue tuning HISM in the current form. The repeated evidence says moving dynamic mobs through UE HISM instance transforms is not the right performance primitive for this setup.

## Stop Condition

Per review, Codex should not pick among the B.13R directions without user direction.
