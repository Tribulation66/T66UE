All five tasks are complete. Summary of the Hero 1 Bounce projectile travel fix:

**Root cause:** `bSuppressTemporaryProjectile=True` on the Bounce binding suppressed the moving visual, so `PerformBounce` presented Bounce via static `ImpactAnchored` slashes spawned at every impact point in the same frame — reading as static slashes / a simultaneous burst, not a travelling chain.

**Fix (presentation only; damage stays authoritative):**
- Removed the static per-link slash spawn from the Bounce path.
- Added `StageBounceProjectileChain` / `SpawnBounceLinkProjectile` to `UT66CombatComponent`: link 0 launches immediately, each later link is scheduled at the running sum of prior links' travel time, so exactly one visual-only `AT66HeroProjectile` (zero damage) is in flight per segment, aimed at the recorded endpoint and self-destructing on arrival.
- Per-link `CombatVFXBounceLinkProjectile` logs added for proof.

**Preserved unchanged:** damage authority, target selection, chain falloff, and the existing `PerChainLink` impact-context publication. No CSV/DataTable/validator/asset edits were needed — the validator's two Bounce assertions still hold.

**Files:** `T66CombatComponent.h/.cpp`, `Hero1AxeBounceMechanismPacket.md` (rewritten from impact-only to moving two-link carrier), `pending_issues_Gameplay.md` (noted the now-dead `bImpactAnchoredCarrier` branch).

**Verification:** Focused `T66Editor Win64 Development` compile **succeeded** (only an unrelated pre-existing Niagara deprecation warning). Report written to `Reports/AgentReviews/Hero1BounceProjectileTravelFix/claude_operator_report.md`.

**Caveat:** This is an Operator artifact, not a greenlight. Visual fidelity is not approved — runtime multi-frame capture and Pablo's approval are still required; I did not run capture (excluded from Operator self-approval), and provided the capture command and expected log evidence in the report.
