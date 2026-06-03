Verdict: APPROVE

## Blockers
- None.

## Major Issues
- None blocking. The plan correctly separates "Codex revise" from "user-only decision" and explicitly preserves PPF/method-substitution and planning-only escape hatches in the APPROVE semantics, which neutralises the largest stated risk.

## Minor Issues
- Verification step 2 reads "Add or run an equivalent direct parse check ... if no existing test exists." Commit to a single path: if absent, add the test; do not leave it optional, otherwise the Codex helper parser regresses silently.
- `decision_block.md` content/schema is not specified for the new `NEEDS_HUMAN_DECISION` flow. Without minimum fields (question, options, blocking-status, packet ref) future continuations may inconsistently "reference the saved gate".
- "Update the accepted process registry row" in `AGENTS.md` doesn't name the registry section. Locate it explicitly in the diff to avoid editing the wrong table.
- The `NEEDS_HUMAN_DECISION` token contains an underscore. Confirm the strict first-line regex matches the literal token and not a broader `\w+`, otherwise novel verdicts could falsely parse.
- The packet uses "PPF" inside the APPROVE exceptions list without restating its expansion. Acceptable if `AGENTS.md` already defines it; flag for the implementer to verify the reference still resolves after the AGENTS edit.
- Implementation order is not stated: helper *parsers* must accept the new verdict before any reviewer *prompt* instructs reviewers to emit it, otherwise a transitional review can produce a valid-looking verdict that fails parsing.

## Clarifying Questions
- For in-flight reviews already on disk that used `REVISE` to mean "human decision", do you want a one-time migration note in `Reports/AgentReviews/`, or forward-only adoption?
- Should `decision_block.md` follow a defined schema, or is freeform Markdown with a heading convention sufficient?
- Does the AGENTS update need to add `NEEDS_HUMAN_DECISION` to any other surface (commit/PR templates, ultrareview helpers) beyond the two review helpers and AGENTS.md?

## Required Verification
- Run `Scripts\Test-ClaudeReviewVerdictParser.ps1` with the new `NEEDS_HUMAN_DECISION` case added; confirm malformed-output fail-closed cases still fail closed.
- Add (don't leave optional) a parser test for `Scripts\Invoke-CodexPlanReview.ps1` covering all four verdicts plus a malformed case.
- `rg` confirmations as listed in the packet, plus an additional sweep for any remaining "go-ahead gate" / "safe to present" phrasing across `Scripts/`, `AGENTS.md`, and `Reports/AGENTS.md`.
- Smoke-invoke each helper against a fixture review output per verdict and confirm the returned object distinguishes `NEEDS_HUMAN_DECISION` from `REVISE` in whatever consumer (Codex automation) reads it.
- After the AGENTS edit, re-read the section to confirm the four verdicts, their routing, and the APPROVE exceptions (planning-only, stop, packet-internal user decision, PPF/method-substitution) are all enumerated in a single place.

## Rationale
The scope is narrow, reversible, and confined to Markdown and PowerShell review helpers — no engine or gameplay surface. The semantic split is the correct fix for the conflation problem and is consistent with the existing `AGENTS.md` greenlight wording. Risks are correctly identified and the exceptions list bounds the auto-implementation change appropriately. Remaining gaps (test commitment, decision-block schema, registry pointer, implementation ordering) are tightenings, not redesigns, and can be addressed during implementation without re-planning.

