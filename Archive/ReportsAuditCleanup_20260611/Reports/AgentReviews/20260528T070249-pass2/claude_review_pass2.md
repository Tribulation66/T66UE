Verdict: REVISE

## Blockers

None that would prevent presentation to Pablo at the go-ahead gate. The plan correctly refuses to fabricate isolation measurements and stays inside AGENTS.md scope.

## Major Issues

1. **Stage 0 binary provenance is under-specified.** The packet says "Run against the existing staged standalone binary if one exists and is runnable" and only mandates rebuild "if no usable staged binary exists." The whole reason B.10 acceptance is open is that the lightweight baseline must reflect post-projectile-manager and post-miniboss-placement source. A pre-existing staged binary on disk does not guarantee it contains those changes. Stage 0 must require a positive provenance check (commit hash / source-build correspondence) before reusing any staged binary, or default to a fresh stage-from-HEAD with the recorded SHA being the B.10 closure artifact. As written, Stage 0 can pass against a stale binary and falsely close B.10.

2. **"One-shot verification log/assert" for `IsActorTickEnabled() == false` is a source change.** The Smoke Validation step proposes "a one-shot verification log/assert that reports IsActorTickEnabled() == false for sampled lightweight mobs." The Default path for both B.11 and B.12 claims "no source code change." Adding a verification log/assert is a source change, even if not per-frame. Either (a) classify this as the "optional source change" path (which has its own approval trigger and rebuild + rehash implications for Stage 0/2), or (b) replace it with an existing automation dump / non-source mechanism (e.g., existing summary line, console command, gameplay-debug overlay) and cite the exact mechanism.

3. **Component-tick audit list incompletely scoped to subclasses.** The audit table is enumerated for `AT66MobBase` and references `BodyHitZone`, `HeadHitZone`, `LockIndicatorWidget`, "any `UCharacterMovementComponent`, custom movement component, timeline, curve helper, or latent per-frame helper attached to lightweight mobs." But the audit asks about "any subclass used by the four lightweight families" without naming those subclass classes. Without an explicit subclass enumeration (Melee/Rush/Flying/Ranged class names + their component sets), the auditor can claim coverage without proving it. Require the audit to first list each lightweight subclass and then walk its component tick state.

4. **Animation evidence escape hatch is loose.** "Multi-frame screenshot sequence OR equivalent runtime evidence" leaves "equivalent runtime evidence" undefined. Given that the explicit anti-lookalike risk is a single frozen frame, define the runtime evidence concretely (e.g., sampled `Frame` scalar parameter values across N captures showing advancement, or a per-clip frame-progression log over a bounded interval — not per-frame).

## Minor Issues

1. The 3-capture floor versus 2-reject halt creates a near-deterministic halt on any post-change set if escalation isn't planned. Clarify that the 3→10 escalation rule applies symmetrically to any post-B.11 or post-B.12 measurement set, not only Stage 0.
2. The narrow docs sweep names only `Gameplay/Combat/MASTER_COMBAT.md` plus "any lightweight-mob master docs found by targeted search." Enumerate the candidate doc paths before go-ahead so the sweep scope is bounded and reviewable.
3. The "B.10.1D runner already records executable fingerprint data" claim is asserted without a path. Cite the file the new runner must mirror.
4. The Stage 0 acceptance includes "expected placed Slime guardian/miniboss route attribution must be documented and explicitly excluded from route-validity rejection." Define what "documented" means in concrete artifact terms — runner config field, README note, or summary table column — to avoid an ambiguous gate.
5. Stage 0 says "zero overhead rejects" while user constraints say reject captures with `PerformanceSystemOverheadMaxUs > 10000` (i.e., individual capture rejection, not pass-level halt until 2+). Reconcile the wording.

## Clarifying Questions

1. Is the staged `T66.exe` currently on disk known to contain post-projectile-manager and post-miniboss-placement source? If not confirmable, will Stage 0 always rebuild from HEAD?
2. Is the proposed `IsActorTickEnabled()` one-shot log/assert acceptable as a non-source-change mechanism, or does it require the optional-source-change approval path (with corresponding rebuild + binary rehash)?
3. Should the lightweight subclass set (Melee/Rush/Flying/Ranged) be named in the audit table before go-ahead?
4. If Stage 0 CVar-on median falls just below the 95% gate but within noise of CVar-off, escalate to 10 captures only, or also flag B.10 as inconclusive even after escalation passes?

## Required Verification

- Source provenance check on whichever staged binary Stage 0 uses, with explicit commit hash recorded alongside the SHA256.
- Component-tick audit table per lightweight subclass with constructor, reset/reuse, and runtime-active states cited by file:line.
- Multi-frame animation proof for each of Melee/Rush/Flying/Ranged covering walk, idle, attack, death/hit-react, and pool-reuse reset.
- Binary hash recorded at pass start, before/after each capture, and pass end — matching the B.10.1D runner shape.
- Concrete runtime proof of `IsActorTickEnabled() == false` for sampled lightweight mobs, using a mechanism whose source-change status is settled per Major Issue 2.
- Stage 0 CVar-off and CVar-on row tables with overhead, route attribution, HeroDeath, and Git/LFS-clean status per capture.

## Rationale

The packet's core move — refusing to reintroduce actor ticking to manufacture B.11/B.12 isolation measurements after finding the code is already in the post-B.12 shape — is correct and well-evidenced (line citations at `T66MobBase.cpp:79-80`, `:897`, `T66MobManagerSubsystem.cpp:678`, `:2149-2228`). PPF, Artifact Parity, Mechanism Manifest, and out-of-scope discipline are all honored. The plan correctly couples Stage 0 to B.10 closure and keeps B.11/B.12 as verification-first with narrowly gated optional source changes. The reason this lands at REVISE rather than APPROVE is two load-bearing ambiguities: Stage 0's binary provenance can silently void B.10 closure, and the proposed `IsActorTickEnabled` verification log is a source change inconsistent with the packet's "no source change" default. Both are tractable with packet edits and do not require re-exploring the live code.

