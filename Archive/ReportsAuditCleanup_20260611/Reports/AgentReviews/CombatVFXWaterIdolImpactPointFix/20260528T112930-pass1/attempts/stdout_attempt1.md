Verdict: REVISE

## Blockers
None.

## Major Issues
- **Trigger condition is under-specified.** The plan says the offset applies "When Hero 1 uses the frontal sector and `EffectiveSlashInnerRadius > 0`", but the code change steps never describe how "Hero 1 / frontal sector" is detected in `PerformSlash`. Is the gate the sector type, the weapon ID, the inner-radius signal, or a combination? This needs to be a concrete predicate before implementation, because the same `SlashImpactContext.ImpactPoint` field feeds every future idol consumer — getting the gate wrong silently shifts other weapons' impact points.
- **`(inner + outer) * 0.5f` is an unsourced approximation.** The diagnosis itself hedges with "roughly midway." For a band with a 0.54 inner-ratio crescent, the visually dominant pixel mass is not necessarily the radial midpoint of the annulus. Either (a) measure against the actual Niagara crescent asset and cite the figure, or (b) explicitly document this as a placeholder approximation that will be revisited when Water Niagara is authored and add a TODO/marker in `pending_issues_Combat.md`. Right now this is a magic number presented as a fix.
- **Vertical offset in the placeholder is unaddressed.** `SpawnWaterIdolImpactPlaceholderVFX` adds `FVector(0,0,76)` on top of `IdolImpactContext.ImpactPoint`. The plan moves the XY of the impact point but says nothing about whether +76Z is still correct at the new XY (band height vs hero-center height may differ). At minimum the plan should state explicitly that Z is intentionally unchanged and why.

## Minor Issues
- **Two files will change, not one.** Step "Edit `Source/T66/Gameplay/T66CombatComponent.cpp` only" contradicts the verification step that runs `git diff --check` against `pending_issues_Combat.md`. List the `pending_issues_Combat.md` edit in the change plan with its intended content.
- **Risks section flags possible proof-target drift but defers no action.** "Current proof target expectations may need to be updated if they assumed the old center" — say whether updating those expectations is in-scope for this change or a follow-up, otherwise the verification capture is ambiguous (does a fail mean the fix is wrong, or that expectations are stale?).
- **Local variable name `SlashContextImpactPoint`** is fine, but the plan should also state that `SlashCenter` is *not* mutated and continues to be used for target gathering, query, and production slash spawn — the change description implies it but doesn't pin it down for review.

## Clarifying Questions
- What is the concrete code-level predicate for "this weapon publishes a band-midpoint impact point" vs. "this weapon publishes the damage center"? Is it derivable from existing weapon data (e.g., `AoeInnerRadiusRatio > 0` plus frontal sector flag), or do you need a new data field?
- Has the 0.5 midpoint been validated against the actual Hero 1 crescent Niagara asset, or is this an eyeballed value?
- Will the existing Water proof target/expected damage center in code or data need to move along with the impact point in this same change, or is that a separate follow-up?

## Required Verification
The listed verification plan is sound (focused compile + Unreal-owned `hero1axeaoewateridolimpact` capture + the five log lines + contact-sheet inspection). Two additions needed:
- Capture or log inspection must confirm the **before/after** XY of `WeaponPrimary` and `IdolPrimary` `ImpactPoint`, not just the post-fix value, so the diff is provable.
- A negative check: capture/log evidence that another AOE weapon (or the same weapon without the gate condition) was *not* perturbed by this change, demonstrating the gate is correctly scoped.

## Rationale
The diagnosis (SlashCenter = damage center ≠ crescent visual impact point) is credible and the architectural choice — fix the published `FT66CombatImpactContext::ImpactPoint` rather than moving Niagara collision or re-enabling the deprecated projectile — is consistent with `CombatVFXIdolOverlayArchitecture.md`, `CombatVFXDefinitionOfDone.md`, and the PPF/Same-Method-Class rule. The seam is the right seam.

The plan does not warrant BLOCK (no hard prerequisite missing) or NEEDS_HUMAN_DECISION (the open items are technical, not product/vision). It does warrant REVISE because two load-bearing details — the exact gate predicate and the source of the 0.5 midpoint — are currently hand-wavy, the Z offset is unexamined, and the file-scope statement contradicts the verification step. These are all Codex-resolvable without user input. Once the gate predicate is concrete, the midpoint is either measured or explicitly marked placeholder, the Z offset is acknowledged, and `pending_issues_Combat.md` edits are listed, this is ready to implement.

