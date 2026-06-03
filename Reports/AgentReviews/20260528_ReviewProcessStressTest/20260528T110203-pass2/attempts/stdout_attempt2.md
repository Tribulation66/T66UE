Verdict: REVISE

Blockers
- None.

Major Issues
- The packet does not state the current regex pattern or whether it is anchored. If the existing pattern is something like `Verdict:\s+(APPROVE|REVISE|...)` without `^...$` anchoring, switching `-match` to `-cmatch` alone may not reject `Verdict: APPROVED` (since `APPROVE` would still match as a substring). The proposed malformed test for `Verdict: APPROVED` will silently fail to exercise what it claims unless the regex is also anchored. Inspect and quote the current pattern, and either confirm it is already anchored or add anchoring as part of the fix.
- Scope of the case-sensitivity change is incomplete. Case-sensitivity matters not just for the token (`APPROVE`) but also for the prefix label (`Verdict:`). Lowercase `verdict: APPROVE` or `VERDICT: APPROVE` will still be accepted under `-match` and still accepted under `-cmatch` only if the literal `Verdict` in the pattern is treated as case-sensitive — which `-cmatch` does enforce, but it must also exactly match the documented `Verdict:`. Confirm the pattern uses literal `Verdict:` (mixed case) and add an explicit malformed test row for `verdict: APPROVE` and `VERDICT: APPROVE`.

Minor Issues
- The verification list captures negative cases (`Verdict: approve` rejected) but does not require a positive regression check that each of the four canonical lines (`Verdict: APPROVE`, `Verdict: REVISE`, `Verdict: NEEDS_HUMAN_DECISION`, `Verdict: BLOCK`) still parses as Greenlit/NotGreenlit correctly after the change. Add explicit positive rows to the parser self-test.
- The packet lists two helper scripts but does not confirm there are no other consumers of the verdict parsing logic (e.g., shared module, packet generator, or stress matrix runner that re-parses output). A quick `rg` for `Verdict:` and the relevant comparison operators across `Scripts/` would close that gap.
- The proposed test additions include `Verdict: NEEDS_USER_DECISION` as stale, which is good, but `AGENTS.md` should be cross-checked to ensure no documentation or template still uses that stale token. If it does, this fix will break a documented example. Add a `rg "NEEDS_USER_DECISION"` sweep to the verification step.
- Out Of Scope is clear, but it does not explicitly say "no changes to AGENTS.md or helper prompts." Since this is a tightening of parser behavior to match docs, the docs should not need to change — but state it.

Clarifying Questions
- None for the user; the open items are all resolvable by Codex inspecting the current regex and call sites.

Required Verification
- Quote the current regex from each helper before editing, and show the post-edit regex.
- Parser self-test passes with: (a) all four canonical verdict lines accepted, (b) `Verdict: approve`, `Verdict: APPROVED`, `Verdict: NEEDS_USER_DECISION`, `verdict: APPROVE`, `VERDICT: APPROVE` all rejected as malformed.
- Stress matrix rerun shows both previously failing rows now reject lowercase, with no new regressions on the other 24 rows.
- `rg` sweep confirming no other script parses verdicts with case-insensitive operators, and no in-repo documentation still references stale tokens like `NEEDS_USER_DECISION`.

Rationale
- The diagnosis is correct and the fix direction (case-sensitive operators) is appropriate and low-risk. However, the packet conflates "case-sensitive token" with "exact verdict line," and the proposed malformed-test additions (`APPROVED`, `NEEDS_USER_DECISION`) only succeed if the regex is also anchored — which the packet never verifies or states. Combined with the missing positive-regression coverage and the unchecked prefix-case scenarios, the plan is one inspection step and a small test expansion away from being safe to implement. Codex can resolve all of this without user input, so REVISE rather than APPROVE or NEEDS_HUMAN_DECISION.

