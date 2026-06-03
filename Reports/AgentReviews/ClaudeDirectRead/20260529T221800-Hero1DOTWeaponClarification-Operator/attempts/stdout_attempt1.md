Understood — leaving that decision with you. To summarize the read-only clarification:

- **One blocking design question** remains open: whether the 3 spheres are cosmetic carriers of the existing single DOT lane (no balance change, my recommended default), 3 independent lanes (~3× damage), or one lane split across 3. Everything else can proceed on stated assumptions.
- **Everything else is non-blocking.** Safest path: reuse the Bounce visual-only seam (`StageBounceProjectileChain`) for a single hero→target link, keep DOT damage resolved at fire time as today, and on arrival spawn 3 placeholder spheres attached to the enemy that pulse on tick for the DOT duration.
- **No code touched** — this is a read-only operator artifact for Codex to validate; the build + Unreal-owned capture proof needs a Codex-approved FullOperator run.

When you pick the damage semantics (or say "go with cosmetic carriers / no clarification needed"), Codex can approve the scope and route implementation.

