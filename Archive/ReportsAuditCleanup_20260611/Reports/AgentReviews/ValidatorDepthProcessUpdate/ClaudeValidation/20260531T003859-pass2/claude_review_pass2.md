Verdict: REVISE

Blockers
- None. Scope is process/tooling only, rollback notes are present per file, and no gameplay/asset/runtime surface is touched.

Major Issues
- **Additive headings risk breaking the strict output contract and the parser.** The packet proposes adding "risk/oversight review" and "assumptions challenged" headings to the deepened prompt while claiming it preserves "existing headings" and the "verdict-line-first contract." But the validator output contract requires a fixed heading set (Blockers / Major Issues / Minor Issues / Clarifying Questions / Required Verification / Rationale). Adding headings can desync the prompt from (a) any "exactly these headings" wording in the protocol/AGENTS and (b) `-ParseReviewPathOnly` if that parser is heading-sensitive. The plan must either fold risk/oversight content *into* existing headings (e.g., Major Issues + Rationale) OR update the prompt, the protocol output template, AND the parser together — and the verification must prove the parser still extracts every heading + verdict after the change. As written the verification only re-parses a deepened review but does not assert the new headings are tolerated or that the canonical heading set is unchanged.
- **Verification gap on the parser-vs-new-headings interaction.** The plan runs `-ParseReviewPathOnly` against the deepened output but does not state the expected parse result (all headings found, verdict found). A pass/fail assertion is needed, not just "dry parse."

Minor Issues
- **AGENTS.md "update only if needed" is undecided.** The plan defers whether AGENTS.md changes at all. Decide before implementation so `AGENTS.md:168` (targeted-by-default wording) does not contradict the new deepened-risk-review affordance. A stale AGENTS.md line is itself a process contradiction.
- **`ExpectedValidatorDepth` is currently `targeted | deepened` (protocol:396-397) but ValidatorBudgetHint reasoning leans on "risk review" language.** Confirm the doc never introduces "risk review" as a third token; the packet says it won't, but the protocol edit text should explicitly state deepened == risk-focused to prevent future drift.
- **Output object field additions** are additive and low-risk, but no consumer inventory was done. A quick grep for callers that index the helper's returned object would de-risk the "downstream tolerates additive fields" assumption.

Clarifying Questions
- Does the existing protocol/helper prompt enforce "exactly these headings"? If yes, are new headings allowed, or must risk content live inside existing headings?
- Should deepened review be auto-enforced for broad packets, or remain opt-in via `-RiskReview`/protocol convention? (Operator flagged this as user-only; see below.)

Required Verification
- Prove canonical heading set is unchanged OR consistently updated across prompt + protocol template + parser.
- Assert `-ParseReviewPathOnly` returns verdict + all expected headings on a deepened review (explicit expected result, not "dry parse").
- Both effort branches: deepened without `-Effort` → effective `high`; deepened with `-Effort low` → effective `low` (good as planned).
- Confirm billing guard path is byte-for-byte preserved and that auto-raise only occurs post-guard on the subscription CLI path.
- Grep for consumers of the helper's returned object to confirm additive fields don't break callers.

Rationale
The plan is well-scoped, keeps a single `deepened` concept, preserves the billing guard, and honors explicit `-Effort`. It is REVISE rather than APPROVE because the headings/parser contract is the load-bearing risk of this exact change and the current verification does not prove the strict output contract and parser survive added headings — resolvable by Codex by either folding risk content into existing headings or updating prompt+protocol+parser together with explicit parse assertions. The auto-enforce-vs-opt-in question is correctly flagged as user-only but does not block implementation of the opt-in path now.

