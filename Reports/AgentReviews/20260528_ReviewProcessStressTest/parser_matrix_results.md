# Parser Matrix Results

| Helper | Case | Expected | Actual | Greenlit | Passed | Note |
|---|---|---:|---:|---:|---:|---|
| Claude | valid_approve | APPROVE | APPROVE | True | True | APPROVE should be the only greenlight. |
| Codex | valid_approve | APPROVE | APPROVE | True | True | APPROVE should be the only greenlight. |
| Claude | valid_revise | REVISE | REVISE | False | True | REVISE is valid but should not greenlight. |
| Codex | valid_revise | REVISE | REVISE | False | True | REVISE is valid but should not greenlight. |
| Claude | valid_needs_human_decision | NEEDS_HUMAN_DECISION | NEEDS_HUMAN_DECISION | False | True | Human decision is valid but must pause. |
| Codex | valid_needs_human_decision | NEEDS_HUMAN_DECISION | NEEDS_HUMAN_DECISION | False | True | Human decision is valid but must pause. |
| Claude | valid_block | BLOCK | BLOCK | False | True | Hard block is valid but must pause. |
| Codex | valid_block | BLOCK | BLOCK | False | True | Hard block is valid but must pause. |
| Claude | missing_verdict | MALFORMED | MALFORMED | False | True | Missing verdict must fail closed. |
| Codex | missing_verdict | MALFORMED | MALFORMED | False | True | Missing verdict must fail closed. |
| Claude | verdict_not_first | MALFORMED | MALFORMED | False | True | Verdict after preface must fail closed. |
| Codex | verdict_not_first | MALFORMED | MALFORMED | False | True | Verdict after preface must fail closed. |
| Claude | unknown_verdict | MALFORMED | MALFORMED | False | True | Unknown legacy/user-gate token must fail closed. |
| Codex | unknown_verdict | MALFORMED | MALFORMED | False | True | Unknown legacy/user-gate token must fail closed. |
| Claude | quoted_verdict | MALFORMED | MALFORMED | False | True | Quoted verdict must fail closed. |
| Codex | quoted_verdict | MALFORMED | MALFORMED | False | True | Quoted verdict must fail closed. |
| Claude | heading_verdict | MALFORMED | MALFORMED | False | True | Markdown heading verdict must fail closed. |
| Codex | heading_verdict | MALFORMED | MALFORMED | False | True | Markdown heading verdict must fail closed. |
| Claude | indented_verdict | MALFORMED | MALFORMED | False | True | Indented verdict must fail closed. |
| Codex | indented_verdict | MALFORMED | MALFORMED | False | True | Indented verdict must fail closed. |
| Claude | lowercase_token_approve | MALFORMED | MALFORMED | False | True | Lowercase token variant must fail closed. |
| Codex | lowercase_token_approve | MALFORMED | MALFORMED | False | True | Lowercase token variant must fail closed. |
| Claude | lowercase_prefix_approve | MALFORMED | MALFORMED | False | True | Lowercase Verdict prefix must fail closed. |
| Codex | lowercase_prefix_approve | MALFORMED | MALFORMED | False | True | Lowercase Verdict prefix must fail closed. |
| Claude | uppercase_prefix_approve | MALFORMED | MALFORMED | False | True | Uppercase Verdict prefix must fail closed. |
| Codex | uppercase_prefix_approve | MALFORMED | MALFORMED | False | True | Uppercase Verdict prefix must fail closed. |
| Claude | stale_approved | MALFORMED | MALFORMED | False | True | Stale APPROVED token must fail closed. |
| Codex | stale_approved | MALFORMED | MALFORMED | False | True | Stale APPROVED token must fail closed. |
| Claude | stale_needs_user_decision | MALFORMED | MALFORMED | False | True | Stale NEEDS_USER_DECISION token must fail closed. |
| Codex | stale_needs_user_decision | MALFORMED | MALFORMED | False | True | Stale NEEDS_USER_DECISION token must fail closed. |
| Claude | trailing_space_approve | APPROVE | APPROVE | True | True | Trailing spaces are tolerated by TrimEnd. |
| Codex | trailing_space_approve | APPROVE | APPROVE | True | True | Trailing spaces are tolerated by TrimEnd. |
| Claude | duplicate_conflicting_after_valid | APPROVE | APPROVE | True | True | Current parser is first-line-only; this exposes a possible stricter future guard. |
| Codex | duplicate_conflicting_after_valid | APPROVE | APPROVE | True | True | Current parser is first-line-only; this exposes a possible stricter future guard. |
