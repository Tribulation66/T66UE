Verdict: REVISE

Blockers
- None. No destructive scope, no unauthorized deletions, and the direction change (deprecate rich basic-mob routing, commit lightweight) is stated explicitly with deprecation-not-delete discipline that matches AGENTS.md guidance.

Major Issues
- Phase 1 reuse hinges on undeclared source equivalence. The packet records the prior staged `T66.exe` SHA `86EDE7D6...` and length, but does not state that the live source tree before this pass starts is byte-equivalent (or behaviorally equivalent for the measured path) to the source that produced those rows. Without that, adopted rows are not a valid Phase 3 comparand. Required: record the source commit/worktree state SHA used at Stage 0a vs. the current pre-implementation source, and reject reuse if they diverge in any file touched by basic-mob routing, VAT state, manager tick, capture profile, or autocapture HP override.
- Phase 1/Phase 3 acceptance is qualitative where it must be numeric. "Median in healthy range," "within noise of Phase 1," and "material regression" are not defined. With no rich A/B comparison, the FPS health check is the only quantitative gate for B.10 closure. Define an explicit lower-bound FPS for Phase 1 acceptance, an explicit relative tolerance for Phase 3 vs Phase 1 (e.g., median delta ≤ X% with overhead within Y us), and an explicit minimum projectile fire/hit count threshold (current `19/19`-`21/21` rows do not anchor a floor).
- DIVORCE/VAT-STATE file disjointness is asserted but not verified. The CVars `T66.Mob.UseLightweight`, `T66.Mob.Diagnostics.RouteFlyingLightweight`, `T66.Mob.Diagnostics.RouteRangedLightweight` are owned by DIVORCE, but their definition site is deferred to "narrow source search before edit." If any of them lives in `T66MobBase.cpp` or `T66MobManagerSubsystem.cpp`, the ownership map collides with VAT-STATE. Resolve the discovery before workstream launch; if they sit in shared files, declare those files main-agent-serialized and split the edits.

Minor Issues
- Build configuration for the combined Phase 3 binary is not specified (Development vs Test vs Shipping, Editor vs Standalone). Capture comparability requires this be pinned and matched to the Phase 1 binary.
- VAT proof spec says "at least three samples ≥0.10s apart per sampled family/clip" but does not require a minimum number of concurrently active mobs per sample, nor a frame-delta threshold to call advancement "evidenced." A pose check or `Frame` parameter delta floor would prevent a near-zero advancement from passing.
- Pool reuse reset evidence is required but the proof method (e.g., log marker on acquire/release with captured pre/post clip+frame) is not described. Define the artifact form.
- "Confirm at most one VAT advancement per active mob per manager tick" lacks a stated proof mechanism. A guard counter or scoped assert in non-shipping that the proof reads from would close this cleanly.
- VERIFICATION workstream constrains itself to a single guardian-kill assertion path for floors 2→3→4, but does not state how the smoke is invoked (cheat command, automation test, console exec) or where its result lands. Pin the invocation and the output sink.
- MINOR-CLEANUP "centralize gambler boss spawn ownership" is the only behavioral change to a rich path in this pass. Call out which path (UI-side or gameplay-side) is being removed, since the choice is product-relevant and not implied by the audit gap.
- Capture hygiene clause says "if overlap, record it and avoid using contaminated FPS rows unless the overlap is demonstrably outside Unreal runtime." "Demonstrably" needs a concrete check (e.g., overlap window does not intersect any accepted capture window) — otherwise this is judgment-by-author.

Clarifying Questions
- Is the source tree currently at the same effective state (for the routing/measurement surface) as it was when the Stage 0a rows were captured, or have edits landed since? This decides reuse vs rerun for Phase 1.
- What numeric FPS floor and Phase 3 tolerance do you want recorded as the B.10 closure thresholds? Without a stated number, B.10 closes on subjective read.
- Where are the three routing CVars defined? If in `T66MobBase.cpp` or `T66MobManagerSubsystem.cpp`, do you want DIVORCE to serialize, or to absorb that CVar edit into VAT-STATE?
- Should the gambler dual spawn path collapse to the gameplay-owned spawn (UI delegates) or to the UI-driven path? AGENTS.md does not implicitly prefer one.
- Does the user want the rich basic-mob deprecation memorialized in `pending_issues_Gameplay.md` as B.10 closure plus a new "rich basic-mob path deprecated" issue, or folded inline into B.10's close-out note?

Required Verification
- Source state equivalence record (commit SHA or worktree diff summary) for Stage 0a baseline vs current pre-implementation state, with a written keep/rerun decision.
- Pre-edit CVar definition-site map for `T66.Mob.UseLightweight`, `T66.Mob.Diagnostics.RouteFlyingLightweight`, `T66.Mob.Diagnostics.RouteRangedLightweight`, `T66.Mob.Diagnostics.UseTouchDamageOverlap`.
- Staged binary build config recorded alongside SHA/length/mtime for both Phase 1 (if rerun) and Phase 3.
- Phase 1 acceptance with numeric FPS floor and fire/hit floor.
- Phase 3 captures with explicit median-delta tolerance vs Phase 1; reject ≥ tolerance triggers VAT-STATE investigation per packet.
- Runtime no-tick proof via `DumpTicks` and per-instance `IsActorTickEnabled()` plus component tick state read at runtime (not header default).
- Multi-frame VAT proof per lightweight family for idle/move/attack/death-applicable clips with frame-delta floor, plus pool-reuse reset evidence with pre/post markers.
- "At most one VAT advancement per active mob per manager tick" verified via in-build counter or scoped assert, with sample recorded.
- Floors 2→3→4 guardian smoke run with descent-hole blocked/unblocked transitions captured.
- Kill-mid-flight source invalidation proof showing dropped or zero-post-death-damage outcome; if bug surfaces, fix recorded within VERIFICATION file set.
- `Hero_2_Chad/AnimatedToonStyle` presence in the Phase 3 staged content confirmed before capture.
- Capture-window hygiene: no `RunUAT`, `UnrealEditor-Cmd`, staged `T66.exe`, or `git-lfs` activity overlap; concurrent main-repo `git add -A` from the other agent recorded with a concrete non-overlap check before each accepted row.

Rationale
- The plan is internally coherent, respects AGENTS.md scope boundaries (Reports routing, no destructive cleanup, owning-doc updates, pending-issue housekeeping), and the PPF/Artifact-Parity/Mechanism-Manifest sections are thorough for the VAT-state move. The product decision to deprecate the rich basic-mob routing path is stated upfront with deprecation-not-delete discipline, which is the right shape under AGENTS.md.
- The reason this is REVISE rather than APPROVE: removing the A/B leaves only an absolute FPS health check as the quantitative gate for B.10 closure, and the packet does not pin numeric thresholds, the build configuration, the baseline-source equivalence claim that justifies reusing the prior rows, or the CVar definition sites that determine whether DIVORCE and VAT-STATE are actually disjoint. These are fixable in the packet without changing direction, but Codex should not implement against them as written.
- Once thresholds, source equivalence, build config, and CVar-site discovery are pinned, this is APPROVE-shaped.

