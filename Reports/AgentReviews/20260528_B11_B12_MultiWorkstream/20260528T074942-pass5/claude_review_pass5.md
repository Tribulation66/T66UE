Verdict: APPROVE

Blockers
- None.

Major Issues
- W2 Task B has a verification path that may be unreachable without an overlay edit. The packet says to "Prefer the existing `T66BossProjectileSmokeKillMidFlight` overlay smoke path without editing `T66PlayerController_Overlays.cpp`," but it does not state that the existing smoke positively exercises a *post-death* `SourceMob` invalidation (vs. just a generic mid-flight kill). If the existing overlay path does not actually drive that branch, W2 is forced into the stop/serialize rule on its first attempted execution. Recommend W2's pre-implementation step include an explicit caller/branch audit of the existing smoke against the `DroppedInvalidSource` increment to confirm reachability without an Overlays edit before the workstream is launched.
- W2 Task A says guardian deaths can be driven through "a tightly scoped automation-only kill helper," but the file home for that helper is not assigned. Helpers cannot live in `Overlays.cpp` (forbidden) and no new harness/test files are allowed without revising the packet. This must either name the W2-owned file that hosts the helper (e.g., add as a non-shipping method on `T66BossBase.cpp` or inside an existing W2-assigned file) or explicitly route the helper through main-agent serialization before W2 starts.
- The neutrality formula `abs(Stage0bMedian - Stage0aCVarOnMedian) <= 2 * max(Stage0aOnStdev, Stage0bStdev)` can swallow a real regression if either stdev is large from `n=3`. The packet escalates to `n=10` when the band is the deciding factor, which is good, but it does not bound the stdev itself. Add an absolute floor or sanity check (e.g., "if `Stage0bMedian < Stage0aCVarOnMedian - X%`, investigate regardless of band") so a noisy capture cannot launder a real perf hit.

Minor Issues
- "All four lightweight families" is referenced in multiple mechanisms but is never enumerated in the packet itself. Listing the families inline (or referencing the authoritative source) makes the smoke acceptance checkable without external context.
- The B.10 gate phrasing "CVar-on median must be at least 95% of CVar-off median" is ambiguous about whether the median is frametime or FPS. The packet inherits this from prior work, but a one-line clarification ("higher FPS / lower frametime / either" with the inequality direction) eliminates a future reviewer reread.
- W3 Task A "add a `BossesChanged` delegate" is allowed only inside `T66ActorRegistrySubsystem.h/.cpp`. If any consumer that needs it lives outside that file set, W3 must stop — but the packet only documents this as a default-no-consumer case. Add an explicit "no broadcast wiring outside the assigned files" line to match the same stop-rule rigor used elsewhere.
- "Bounded `Frame` scalar dump from an existing or approved one-shot hook" should explicitly cap sample count (e.g., the three-sample minimum is also the maximum unless approved) so the multi-frame proof does not slide into per-frame logging.
- Phase 0 instructs verifying `ANTHROPIC_API_KEY` absent from Process/User/Machine scopes; if the operator does this via PowerShell, it must not echo the key value during checks. Worth one explicit line.
- The deleted `Content/...` enumeration in the worktree classification is well-specified; verify the table requires *path-by-path* Pablo decisions, not a single "all deletions: non-runtime" lump decision. The current wording allows lumping if evidence is provided; tighten if Pablo prefers per-path.

Clarifying Questions
- Where will W2's "tightly scoped automation-only kill helper" live, given Overlays is off-limits and new harness/test files are forbidden?
- Has the existing `T66BossProjectileSmokeKillMidFlight` been confirmed to drive source-boss invalidation (death/destroy) while a projectile is still active, or does it only kill mid-flight via projectile lifetime/HP, leaving the `SourceMob != live boss` branch un-exercised?
- For the runtime tick proof, is `DumpTicks` already invokable via existing automation hooks at the moment lightweight mobs are live, or is a hook addition expected? If the latter is already likely, surfacing it now lets Pablo decide on the SHA-rerun cost before implementation begins.
- Is the actor-side `ForceMobVertexAnimationClipForAutomation` wrapper expected to compile under shipping builds, or is it `#if WITH_T66_AUTOMATION` / non-shipping guarded? If non-shipping only, confirm the focused-compile target exercises the path.

Required Verification
The packet already lists these — repeating only because the verdict depends on them all firing:
- Worktree classification table written, all paths decided, before staging.
- Stage 0a CVar-off and CVar-on each accepted at `n=3`, hash-stable, with stdevs recorded.
- Focused compile after W1 (or recorded exception + combined compile post-integration).
- Post-W1 source audit explicitly asserting `AT66MobBase::ForceMobVertexAnimationClipForAutomation` body writes zero actor VAT fields.
- Stage 0b CVar-on at `n=3`, escalated to `n=10` if the noise band is the deciding factor.
- `DumpTicks` (or Pablo-approved hook, with SHA rerun) proving no `AT66MobBase` actor tick and no lightweight component tick while mobs are live.
- Multi-frame VAT proof: ≥3 samples ≥0.10s apart for all four families, plus pool-reuse reset evidence.
- Floors 2/3/4 placed-guardian gating proof on the combined binary.
- Positive `SourceMob`-invalidation proof for boss projectile mid-flight drop.
- B.10 close edit in `pending_issues_Gameplay.md` recorded against the final closure SHA, not Stage 0a's SHA, if any hook was added after Stage 0a.

Rationale
The packet is unusually rigorous: it corrects the obsolete B.11/B.12 premise against live source (actor tick already disabled at `T66MobBase.cpp:79–80,897` and manager-driven VAT calls in `T66MobManagerSubsystem.cpp:2149–2228`), narrows scope to the residual actor-resident VAT state in `T66MobBase.h:268–276`, and explicitly stays out of B.13 HISM/per-instance custom data. File ownership is disjoint and `T66PlayerController_Overlays.cpp` is firewalled to main-agent serialized scope with stop/serialize rules for both W2 and W3. Worktree classification is mandatory before staging, B.10 closure is properly deferred until after the runtime tick proof and hook decision, capture policy preserves the 10ms overhead reject / HeroDeath halt / binary-hash gates, and the `ForceMobVertexAnimationClipForAutomation` wrapper preserves external automation surface so W1 does not silently expand its file set. The major issues identified are addressable through pre-launch audits and one explicit file assignment rather than structural rework, so the packet is safe for Codex to present to Pablo at the existing AGENTS.md go-ahead gate — APPROVE does not waive that gate.

