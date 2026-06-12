Verdict: REVISE

## Blockers

None.

## Major Issues

- **`claude auth status` output format assumption is unverified.** The preflight expects to parse `"loggedIn": true`, `"authMethod": "claude.ai"`, `"apiProvider": "firstParty"` as JSON, but elsewhere the packet describes the same command as "reports first-party claude.ai subscription login" (text). If the CLI outputs human-readable text by default, the fail-closed regex/parse must be specified, or the call must pin `--output-format json` (if supported by `auth status` — not confirmed in the help sweep). Pick one contract and verify before writing the preflight; otherwise this either fails closed on a valid session or silently passes a malformed one.
- **`Invoke-ClaudeDirectRead.ps1` default mode is `Operator`.** Operator mode skips strict verdict semantics and runs `--effort high`. Defaulting to the looser, more expensive mode means a forgotten `-Mode Review` silently downgrades verification rigor and burns Opus 4.8 budget. Either flip the default to `Review`, or make `-Mode` mandatory (no default). Document the rationale in the helper header either way.
- **Reports artifact convention not propagated.** The plan writes Claude direct-read artifacts to `Reports/AgentReviews/ClaudeDirectRead/<timestamp>-<task>/`, but `Reports/AGENTS.md` and `Reports/README.md` are not in the edit list. New agents will not discover this path or its retention/rotation rules. Add a Reports-side doc update so the artifact location is registered.

## Minor Issues

- **No timeout specified for `Invoke-ClaudeDirectRead.ps1`.** `Invoke-ClaudePlanReview.ps1` has timeout retries; the new helper should inherit equivalent behavior or explicitly justify omission, especially in `Operator` mode at `--effort high`.
- **Subsection name `Claude/Codex Operator Stack`** lives near `Claude Cross-Review` but the packet does not show the exact heading hierarchy or registry row format. Risk of structural drift in `AGENTS.md`. Recommend Codex draft the literal block (heading level, bullet wording, registry row format) and re-review before edit, since AGENTS.md governance is process-critical.
- **`--add-dir C:\UE\T66`** is the entire repo. With read-only tools that is acceptable, but worth an explicit AGENTS line: "Claude direct-read scope is whole-repo read; sensitive paths must be excluded if added later." Future-proofs against tool widening.
- **No mention of how `Invoke-ClaudePlanReview.ps1` parameter additions interact with existing callers** (e.g., other scripts, automation). Defaults preserve behavior except for the new auth preflight, which could fail-closed runs that previously passed. Acceptable, but call it out.
- **Blender MCP step nested inside the same review pass** but gated "after review approval." Cleaner to split: Codex applies docs/scripts changes under this APPROVE, then a separate small-scope approval for the MCP config write. Reduces blast radius of a single approval.
- **Out-of-Scope list is solid but does not mention** "no Claude editor automation," "no Unreal Python invocation by Claude." Worth listing explicitly given that Unreal/Niagara appears in the AGENTS update.

## Clarifying Questions

- For `auth status`: does the local Claude CLI version installed on this machine support `--output-format json` for the `auth status` subcommand specifically, or only for the main `claude` invocation? This determines whether the preflight is text-regex or JSON-parse.
- Should the Accepted Process Registry rows include effective date, owner, and rollback note, matching whatever schema existing rows use? Packet does not show.
- Should `Operator` mode emit a one-line "OPERATOR ARTIFACT — NOT A GREENLIGHT" banner in the run manifest so downstream agents do not mistake it for a verdict?

## Required Verification

- After implementation, `git status` immediately after each `Invoke-ClaudeDirectRead.ps1` smoke run to confirm zero working-tree changes (proves read-only tool restriction held).
- `Test-ClaudeReviewVerdictParser.ps1` passes unchanged.
- `Invoke-ClaudePlanReview.ps1` smoke run returns object containing `Model=claude-opus-4-8`, `Effort`, and `ClaudeVersion`.
- Negative-path preflight test: temporarily unset/login-out auth (or simulate) and confirm helper fails closed without saving PII.
- `claude mcp list` and `claude mcp get blender` outputs captured (sanitized) in the done report.
- `AGENTS.md` diff reviewed by Claude in a separate small packet before the registry rows are considered authoritative.

## Rationale

The plan correctly defers denominator/quota work, preserves the cross-validation invariant, keeps Claude tool surface read-only, separates Operator vs Review profiles, and adds an auth-method fail-closed guard. The structural direction is right and the risk register is realistic.

What blocks APPROVE is implementation-detail ambiguity that would otherwise be discovered mid-edit: the auth-status parse contract, the default-mode safety inversion, and the missing Reports-side documentation update. These are all Codex-resolvable — none require user product direction or external state — so REVISE rather than NEEDS_HUMAN_DECISION or BLOCK. Address the three major issues, tighten the verification plan to include a post-run `git status` and a negative-path auth test, and resubmit a small revised packet (the AGENTS.md literal block can be split into its own micro-review if convenient).

