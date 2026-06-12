`git diff --check` is clean except the known LF/CRLF warning. Done.

## Summary

Made the minimal docs/process changes inside the Codex-approved scope:

- **`AGENTS.md`** (Section 1) — added three bullets after the existing decision-gate wording: a Decision Gate Stop rule (first valid `NEEDS_HUMAN_DECISION`/equivalent = immediate stop, with explicit no-do list), a continuation rule (pre-answer turns only reference the saved decision block + choices), and a host-policy carve-out (if the goal tool can't mark `blocked` immediately, still stop and state the decision-gated status in conversation).
- **`OPERATOR_VALIDATOR_PROTOCOL.md`** — added a `### Decision Gate Stop Rule` subsection right after the verdict meanings, mirroring the same rule for Operator and Validator.
- **`completion_packet.md`** — wrote the packet, including the B.13 failure-mode statement (should have stopped immediately and only referenced the saved decision block on continuations) and validation suggestions.

All four implementation requirements plus the protocol mirror are covered. `git diff --check` on the touched docs is clean except the expected LF→CRLF warning. No runtime source, assets, or Git state touched.

This is an Operator work artifact, not a greenlight — it's ready for Codex to validate the actual diff and write the final report.

**Codex Token Spent:**
0

**Claude Tokens Spent:**
Unavailable

**Tier: Tier 1**
