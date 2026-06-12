# Approval Process Update Plan Packet

Working goal: update the repo approval process so an exact Claude `Verdict: APPROVE` authorizes implementation without requiring an additional manual Pablo go-ahead, then update the active B.11/B.12 multi-workstream packet to match.

Output scope: review of this process-doc change and active-pass packet amendment only.

## Context

The current root process requires two gates for non-trivial work:

1. external review greenlight with first non-empty line exactly `Verdict: APPROVE`
2. separate explicit Pablo go-ahead after review approval

The user has now requested a process change:

- If Claude approves, Codex can go straight ahead with implementation.
- Apply the same rule to the active B.11/B.12 pass-specific packet.
- In the final B.11/B.12 combined report, note that manual Pablo approval after a clean review is no longer required.
- Treat an unrelated `git add -A` process as another agent's user-requested work and ignore it rather than killing it.

This packet intentionally applies the auto-proceed rule to Claude approval only. If Claude is unavailable and the separate local Codex fallback reviewer is used, the old manual-confirmation step remains in force unless the user explicitly changes that later.

Bootstrap note: this packet is still being reviewed under the old root `AGENTS.md` rule. The user's request in this turn is the Pablo go-ahead for this specific meta-change after Claude returns `Verdict: APPROVE`. If Claude does not approve, Codex revises or stops as normal.

## Applicable Instructions

- `C:\UE\T66\AGENTS.md`: root process router and Claude review policy.
- `C:\UE\T66\Reports\AgentReviews\20260528_B11_B12_MultiWorkstream\plan_packet.md`: active B.11/B.12 packet with a stricter "Pablo go-ahead" clause.
- `C:\UE\T66\Reports\AGENTS.md`: report artifacts under `Reports/`.

## Proposed Edits

### Root `AGENTS.md`

Change the Claude Cross-Review section so that a valid review greenlight authorizes implementation without an additional manual Pablo go-ahead.

Preserve these safeguards:

- Claude review remains the default unless explicitly skipped, unavailable, or impossible.
- `ANTHROPIC_API_KEY` must still be absent before using Claude review.
- The exact first non-empty line must still be `Verdict: APPROVE`.
- Malformed review output still fails closed.
- Blocker/Major objections still require revision or a user decision.
- If all external reviewers are unavailable, Codex must still ask whether to proceed without external review.
- User-specified holds, planning-only boundaries, or "do not implement yet" instructions still override.
- Codex-fallback reviewer approvals do not auto-proceed; they still require manual user confirmation unless a future process update explicitly says otherwise.

Replace the "wait for user's explicit go-ahead" requirement with:

- After Claude review approval, report the review artifact, reviewer, caveats, and the implementation start scope; then proceed unless the user explicitly asked for a hold or the approved packet requires a further product decision.
- A "further product decision" must be explicit, not inferred. Valid markers are: a packet section headed `Product Decision Required`, an unresolved reviewer Blocker/Major that identifies a user-only decision, an unresolved `Clarifying Questions` item that changes implementation scope, or a direct user instruction that the work is planning-only or should stop before implementation.

### Active B.11/B.12 Packet

Update `Reports/AgentReviews/20260528_B11_B12_MultiWorkstream/plan_packet.md` to remove or amend the packet-local "Pablo go-ahead after Claude review" requirement.

Preserve:

- mandatory dirty-worktree classification before Stage 0a
- path-by-path classification and halt on unknown runtime-affecting paths unless a safe non-destructive isolation path is available
- no revert/stash/clean/reset/discard of user changes without explicit path action approval
- Stage 0a before implementation
- binary-hash stability and capture hygiene

Add a note to the packet/final report obligations:

- "Manual Pablo go-ahead after an approved Claude review is no longer required by root process. Claude approval now authorizes implementation, unless a packet or user instruction explicitly marks the work planning-only or requires a separate product decision. Codex-fallback approval still requires manual confirmation."

## Implementation Plan

1. Run this plan packet through `Scripts\Invoke-ClaudePlanReview.ps1` after confirming `ANTHROPIC_API_KEY` is absent.
2. If the review is `Verdict: APPROVE`, patch `AGENTS.md`.
3. Patch the active B.11/B.12 plan packet to match the updated approval model.
4. Continue the B.11/B.12 pass using the updated process.
5. Include the process change in the final combined B.11/B.12 report.

## Risks

- Risk: removing the manual go-ahead weakens user control.
  - Mitigation: review approval remains required, and explicit user holds/planning-only boundaries still stop implementation.
- Risk: some work requires a product decision after review.
  - Mitigation: keep a separately marked exception for unresolved product/scope decisions that only the user can make. The marker must be explicit as defined above.
- Risk: dirty-worktree classification could be mistaken as removed.
  - Mitigation: this change only removes the extra post-review manual go-ahead. It does not remove source-provenance or measurement gates.
- Risk: B.11/B.12 was originally approved under the old two-gate rule.
  - Mitigation: the user explicitly requested the mid-pass process update in this turn, this packet is reviewed before patching, and the final combined B.11/B.12 report will record both the user request and the Claude approval artifact.
- The `git add -A` process note is session-local only. It is not a proposed repository policy edit.

## Verification

- `rg` confirms `AGENTS.md` no longer requires a manual go-ahead after a clean Claude review.
- `rg` confirms the active B.11/B.12 packet no longer says implementation must wait for Pablo go-ahead after Claude review approval.
- `rg` confirms the exact `Verdict: APPROVE` requirement remains present.
- `rg` checks exact removal/replacement phrases: `"Pablo go-ahead"`, `"explicit go-ahead"`, and `"wait for user"`.
- `rg` confirms required preserved clauses still exist: `"ANTHROPIC_API_KEY"`, `"Malformed Claude verdict output is not a Claude availability failure"`, and `"both unavailable"` / `"ask whether the user wants to continue without external review"`.
- `rg` confirms B.11/B.12 packet still contains dirty-worktree classification before Stage 0a, path-by-path classification, halt-on-unknown runtime-affecting paths, no revert/stash/clean/reset/discard without explicit path action approval, Stage 0a before implementation, binary-hash stability, and capture hygiene.
- B.11/B.12 final combined packet includes a section noting the process update.
- The final report quotes the actual patched `AGENTS.md` language rather than paraphrasing it.

## Reviewer Questions

Please review for contradictions with existing repo process, unsafe removal of user-control gates, accidental removal of Claude/Codex-fallback review requirements, and whether this patch can be safely implemented after a clean review without further user approval given the user's explicit request in this turn.
