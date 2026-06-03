Verdict: APPROVE

Blockers
- None.

Major Issues
- None. The answer correctly establishes (1) the AOE weapon was equipped via real combat, (2) the visible "weapon effect" is the production `Hero1Axe_AOE_Base` Niagara slash, and (3) the absent blue projectile is the intentionally suppressed deprecated temporary projectile per `bSuppressTemporaryProjectile=True` in `CombatVFXBindings.csv`. All three are backed by log lines and source citations in the packet.

Minor Issues
- The "frames 45-49" overlap window is slightly loose given the packet's own frame-by-frame notes: frame_0044/45 are pre-impact, the readable slash is mainly at frame_0047 with faint tail at frame_0048. A tighter 46-48 selection window would more accurately reflect the evidence, though 45-49 as a video-quality lead-in/tail-out is defensible.
- The answer leans on "structural/log proof only" framing, which is correct per `CombatVFXDefinitionOfDone.md`, but it could be more explicit that the existing capture is still acceptable as the logged-behavior proof for the Water fallback placeholder branch — i.e., what fails is visual acceptance of the weapon-underneath claim, not the idol overlay proof itself.

Clarifying Questions
- None required for Codex to proceed under the proposed scope (re-select frames, pair with existing `hero1axeaoevfxbinding` Baseline, no behavior or harness changes). The user's question is a clarification request, not a directive to change runtime behavior.

Required Verification
- Re-deliver contact sheet selecting frames in the 46-48 (or 45-49) window from the existing `Hero1AxeAOE_WaterIdolImpact_UserVideo_20260528` capture so the production Niagara slash is readable in at least one selected frame.
- Deliver or point to the existing `hero1axeaoevfxbinding` Baseline capture output and present it side-by-side with the Water idol capture so the "weapon-only vs weapon+Water idol" comparison is visible.
- Confirm `pending_issues_Combat.md` entry "Water Idol Impact Capture Weakly Shows Base Weapon VFX [Minor]" is present and links to this answer for traceability.
- No re-run of capture or code change is required for this answer to land; if Codex later wants visually stronger proof, that would be a separate scoped task and should not be bundled here.

Rationale
- The packet directly answers the user's concern: the weapon was equipped, the idol fired at the weapon impact point, and the apparent "missing projectile" is the deprecated temporary projectile that the production binding intentionally suppresses. The fix is capture-selection plus pairing with the already-existing weapon-only baseline, which respects `CombatVFXIdolOverlayArchitecture.md` (weapon VFX owns the primary silhouette) and `MASTER_COMBAT.md` (production-path automation proof). Pass 1's correct objection to delaying/hiding the Water placeholder was honored — the revised draft does not propose any behavior change that would compromise the Water fallback-branch proof. No unsafe re-enabling of deprecated visuals is suggested. Scope is read-only documentation/capture-selection work, safe for Codex to proceed.

