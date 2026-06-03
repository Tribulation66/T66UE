Only a docs reference remains in `pending_issues_Gameplay.md` (not code). The fix is complete.

## Summary

**Root cause:** The Bounce carrier routed the authored Niagara slash through a runtime `SetAsset()` + `Activate()` on a pre-created default-subobject `UNiagaraComponent`. That path rendered **no visible particles in any frame** — confirmed across the full prior capture (hero, enemies, and damage numbers visible, but no slash). The earlier orientation hypothesis was wrong: the link segments are horizontal (`Z=64` constant), so velocity-pitch was negligible.

**Fix (smallest faithful correction):** Spawn the authored slash exactly as the proven first-iteration path does — `UNiagaraFunctionLibrary::SpawnSystemAtLocation` (which returns a fully initialized component) — then drive that free component's world location along the link each tick via the visual-only mover. This restores the first-iteration look as a *moving* carrier without substituting the method class. Removed the dead subobject path.

**Result (verified):**
- Compile: `Result: Succeeded`.
- `PROOF_carrier_travel_39-43.png` (accepted 12fps): frame 39 no arc → 40 damage 28 lands → 41 slash arc appears over primary → 42 arc traveled right toward second → 43 dissipated. The authored slash is now visible and moving.
- Logs: `LinkIndex=0`→`LinkIndex=1`, `LinkCount=2`, no `LinkIndex=2`, both carriers = `NS_Hero1AxeBounce_MeshSlash`, arrival callback fired, damage preserved (primary 28, second 26).

Operator packet written to `Reports/AgentReviews/Hero1BounceVisibleCarrierFix/claude_operator_report.md`. Codex tokens this turn: 76,672. No git actions taken (excluded by scope).

Files changed: `T66CombatComponent.cpp`, `T66HeroProjectile.cpp`, `T66HeroProjectile.h`.
