Verdict: REVISE

Blockers
- None.

Major Issues
- The recommendation to "reduce or delay the Water placeholder for proof" conflicts with what `CombatVFXIdolOverlayArchitecture.md` says Water's first proof is validating: the fallback placeholder branch spawning at the weapon impact point. Delaying or hiding that placeholder to expose the weapon slash visually misrepresents the architectural behavior being proved (placeholder firing on the WeaponPrimary impact point). The answer should distinguish "make the weapon slash readable" (acceptable: timing/frame selection/side-by-side base capture) from "delay the idol placeholder" (problematic: alters the very thing under test).
- The proposed fix path bundles three different kinds of change (frame reselection, placeholder timing change, harness proof-only visibility mode) without saying which Codex will actually do, and which require a Plan under the combat VFX process docs. The DoD says gameplay capture must be Unreal-owned and that frame selection alone does not prove fidelity, so any "proof-only visibility mode" or placeholder timing change is a combat-VFX behavior change that needs its own planning loop, not a one-line "best correction."

Minor Issues
- The draft jumps from "the AOE slash is very brief" to "select frames around 45-48" without naming the root cause: the production Niagara slash duration vs. the placeholder spawn delta at the shared `ImpactPoint=V(360,64)`. A one-line statement of the actual temporal overlap (slash window vs. placeholder spawn frame) would make the recapture spec concrete.
- The answer cites `bSuppressTemporaryProjectile=True` correctly but does not explicitly call out that the user's "I don't see a projectile" expectation is partly a mental-model mismatch (AOE primary is a slash, not a traveling carrier). Naming that mismatch directly would prevent the same question recurring on the next idol capture.
- "Side-by-side base-only Hero 1 AOE capture plus base+Water idol capture" is a good idea but is not tied back to an existing capture harness mode or a new one — unclear if `hero1axeaoebaseonly` exists or needs to be added (and therefore planned).
- Does not reference `Gameplay/Combat/pending_issues_Combat.md` to record the weak-proof finding, even though that is the project's standard place to log this kind of follow-up.

Clarifying Questions
- Is the user asking for (a) confirmation the system is correct and a better proof artifact, or (b) a behavioral change to the AOE so a moving carrier is visible? The draft assumes (a); confirm before recapture work begins.
- Is altering the Water placeholder spawn timing in proof mode acceptable to the user given the architecture doc states the placeholder branch is what's being validated for Water's first proof?
- Should the recapture be scoped to this branch (alpha-0.8 era) or deferred until the real Water idol Niagara asset replaces the BlueSphere placeholder, at which point this proof becomes obsolete anyway?

Required Verification
- Re-read frames 44–49 of `Saved/VideoCaptures/Hero1AxeAOE_WaterIdolImpact_UserVideo_20260528/frames/` and state the exact frame range where the production slash is visible vs. when the placeholder first appears, so the recapture spec has a measured overlap window, not an estimate.
- Confirm in `Scripts/CaptureT66GameplayVideo.ps1` and `T66PlayerController_Overlays.cpp` whether a base-AOE-only capture mode already exists; if not, flag the new mode as a planning item, not part of this answer.
- Cross-check `CombatVFXDefinitionOfDone.md` — confirm that side-by-side captures and adjusted frame selection are sufficient DoD evidence for the "idol on top of weapon" claim, or whether a fresh DoD-compliant capture run is required.
- Verify `pending_issues_Combat.md` is updated with the weak-visual-proof finding so the follow-up does not get lost.

Rationale
The core technical claims are accurate and well-evidenced: logs prove the AOE weapon was equipped and fired through the real production path, the shared impact point connects WeaponPrimary → IdolPrimary, and `bSuppressTemporaryProjectile=True` correctly explains the absent legacy projectile. Refusing to re-enable the deprecated temporary projectile is the right call and aligns with `CombatVFXIdolOverlayArchitecture.md`. However, the corrective recommendation as written risks undermining the architectural proof itself (by suggesting placeholder delay) and conflates frame-selection fixes with capture-harness behavior changes that the combat VFX process docs require to be planned separately. Codex can resolve this by tightening the fix path into "what's a reselection vs. what's a new planned harness/capture change," dropping the placeholder-delay suggestion (or explicitly justifying it against the architecture doc), and naming exactly what artifact will replace the current video.

