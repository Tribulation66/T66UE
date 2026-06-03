Verdict: REVISE

## Blockers
None — the structural premise (live actor tick already disabled, move only VAT state ownership; Stage 0a-before-implementation; disjoint file map; explicit Overlays.cpp assignment to W3) is sound and aligns with the live code observations cited.

## Major Issues
- **W1 external caller enumeration is missing.** The plan replaces actor-side `SetMobVertexAnimationClip`, `ForceMobVertexAnimationClip*`, and the VAT helper API with manager calls, but does not enumerate the call sites of those actor APIs across the source tree. If any caller lives outside `T66MobBase.*`/`T66MobManagerSubsystem.*` (e.g., a combat/status/automation component), that file becomes implicit W1 scope and breaks the disjoint ownership map. This audit must be a pre-implementation step recorded against the W1 file set, not discovered mid-execution.
- **W2 smoke/harness file location undefined.** "Add or reuse a non-shipping traversal smoke" and "force or schedule a boss projectile" typically require automation/test files (e.g., `*.spec.cpp`, gauntlet/automation classes) that are *not* in W2's exclusive set. The packet must either (a) constrain W2 to existing harnesses with no new files, or (b) explicitly enumerate the test/automation files W2 may add, otherwise W2 will either stop or silently expand scope.
- **B.10 closure ordering vs runtime tick proof.** Stage 0a passing closes B.10 with the Stage 0a SHA inline in `pending_issues_Gameplay.md`, but the runtime no-tick proof (and the contingency that a one-shot hook may be required, which invalidates that SHA) only happens on the Stage 0b binary. Closing B.10 against the 0a SHA before the runtime proof is collected risks an immediate edit/revert of the pending-issues entry. Recommend deferring the actual `pending_issues_Gameplay.md` close edit until after the runtime tick proof on the closure binary is collected, even when 0a passes the perf gate.
- **`UT66CharacterVisualSubsystem` boundary unspecified.** W1 says `ApplyConfiguredVisual` "may still call the visual subsystem and obtain a row/MID" and that MID ownership moves to the manager. If the visual subsystem currently sets actor-resident state (writes the row/MID/clip into `AT66MobBase` fields), the handshake change is an interface change on the visual subsystem. The plan should explicitly confirm whether `UT66CharacterVisualSubsystem.*` requires any edit; if yes, it must be added to W1's file set (or main-agent serialized), not left ambiguous.

## Minor Issues
- **Parallel-without-per-workstream-compile risk.** The packet allows skipping per-workstream compile if "delegated tooling makes per-workstream compile impractical." Three uncompiled workstreams merged at once is a known integration footgun; a per-workstream compile gate should be the default, with the impractical-tooling escape requiring an explicit main-agent note.
- **Final proof directory path is unspecified.** W2 says "save log/screenshot evidence under the final pass proof directory" but does not name it. Pin it (e.g., `Reports/AgentReviews/20260528_B11_B12_MultiWorkstream/proofs/...`) so workstreams don't invent divergent locations.
- **Floor 2 guardian proof is implicit.** The traversal smoke covers floors 2/3/4 in narrative, but Task A's named asserts only itemize "each placed slime guardian" generically — make the floor-2 guardian gate explicitly part of the assert list, since the world reference gates floors 2/3/4.
- **`ForceMobVertexAnimationClipForAutomation` post-refactor audit.** The packet correctly requires the source audit to verify zero actor-resident state, but doesn't say what happens if the symbol's non-shipping convenience cannot delegate without storing transient state — add a stop/escalate rule rather than an implicit pass.
- **Worktree classification scan scope.** "Narrow status/diff checks for the files relevant to this pass and the dirty categories already flagged" is fine, but the dirty worktree in the live status also touches `Source/T66/Core/RunState/T66RunStateSubsystem_Combat.cpp` and the Weapons DT/CSV — make sure the classification table explicitly includes those runtime-affecting source/data files, not only the deleted `Content/...` deletions.

## Clarifying Questions
1. Are there any existing callers of `AT66MobBase::SetMobVertexAnimationClip` / `TickMobVertexAnimationState` / `ForceMobVertexAnimationClip*` outside the W1 exclusive file set today? (This should be answered with a grep before sub-agents launch.)
2. Does the W2 smoke harness already exist in a file W2 is permitted to edit, or will W2 need to add new automation files? If new, which file(s)?
3. Does `UT66CharacterVisualSubsystem::ApplyMobVertexAnimationVisual` currently write into `AT66MobBase` actor fields, or does it return a row/MID that the actor stores? The handshake change depends on this.
4. Is `pending_issues_Gameplay.md` allowed to be edited a second time (to swap the SHA) if a one-shot hook is added post-0a, or does Pablo want a single atomic close after all proofs land on the final closure binary?

## Required Verification
- **Pre-implementation**: enumerate every caller of the actor-side VAT API and confirm all live within W1's exclusive set, or expand/serialize before parallel launch.
- **Worktree classification artifact** at the specified path, including runtime-affecting source/data dirties (RunState, DT_Weapons, Weapons.csv) and deleted `Content/...` assets, with per-path Pablo decisions captured before staging.
- **Stage 0a**: 3+3 captures, binary hash invariance, overhead < 10 ms, no HeroDeath, B.10 perf gate evaluated but pending-issues edit deferred until runtime tick proof lands.
- **Per-workstream focused compile** (default) before integration; impractical-tooling exception only with explicit main-agent note.
- **Stage 0b**: combined-binary CVar-on neutrality vs Stage 0a CVar-on within `2 * max(stdev)`.
- **Runtime tick proof** via `DumpTicks` or Pablo-approved hook; if hook is added post-0a, rerun closure measurement and update the B.10 SHA before any close edit lands.
- **Multi-frame VAT proof**: ≥3 samples ≥0.10 s apart, all four families, at least one death sample, pool reuse reset evidence.
- **Floors 2/3/4** guardian-blocks-then-opens evidence on the descent hole; floor-4 guardian gates boss-floor entrance.
- **Kill-mid-flight** positive exercise of `DroppedInvalidSource` branch with no post-death hero damage.
- **Source audit** showing actor-resident VAT fields removed and manager owns the data + helpers; at-most-one advancement per active mob per tick still holds across the eight manager branches.

## Rationale
The plan correctly resets the B.11/B.12 premise against live code (actor tick already disabled, manager-driven advancement already wired, placed miniboss + projectile manager already in source), constrains scope to manager-owned VAT state without leaking into B.13 HISM/custom-data work, and lays out a disjoint file map with explicit serialization rules and an unambiguous Overlays.cpp assignment. The Stage 0a-before-implementation measurement is the right way to ground B.10 closure. The remaining gaps are concrete and addressable without restructuring the plan: (1) audit and lock W1's external caller surface, (2) name the W2 harness location, (3) defer the B.10 pending-issues edit until the runtime proof lands on the closure binary, and (4) confirm the `UT66CharacterVisualSubsystem` boundary. With those resolved, the packet would be safe to present to Pablo at the go-ahead gate.

