Verdict: APPROVE

Blockers
- None. This is an answer-only research packet with no implementation requested; nothing here commits the repo to a risky or irreversible change.

Major Issues
- Anthropic OAuth usage endpoint (`api.anthropic.com/api/oauth/usage`) is treated as a working data source but is undocumented/unofficial. The packet flags this correctly and proposes the documented Claude Code statusline JSON as the stable fallback. Acceptable for research, but any future implementation must default to the statusline contract, not the OAuth endpoint.
- Tool-evidence consistency: "AgentWatch" appears in Findings #4 (Windows list) but is not in the Research Sources section, and "SessionWatcher/AIHelper" are listed without primary links. For a research deliverable this is tolerable, but the citation list and the findings list should match before final output.

Minor Issues
- The `19%` (screenshot) vs `18% remaining` (probe) reconciliation is presented as confirmation; it is close but not exact. Worth stating that the small delta is expected drift between screenshot time and probe time, not a discrepancy.
- "Proposed Final Answer" repeats the build-step list already in Findings #5; could be trimmed.

Clarifying Questions
- None required for the research scope. (Build-vs-use is correctly deferred as a product decision and does not block delivering the research answer.)

Required Verification
- None beyond what was done. The local probes (Claude OAuth usage response, Codex `account/rateLimits/read`) are the appropriate verification for a research claim and were performed read-only. Token values were confirmed not printed.

Rationale
The packet's working goal is research/answer-only and it delivers exactly that: it confirms both values are obtainable via structured local paths (Codex `app-server` + `account/rateLimits/read`; Claude statusline JSON, with the OAuth endpoint as an unofficial alternative), backs claims with primary OpenAI/Anthropic docs first per AGENTS, surveys existing tools, and includes sound security caveats (no token storage/printing, modest polling, OCR as last resort). No implementation is proposed for execution, so there is no unsafe scope to gate. The only follow-ups (citation/list consistency, defaulting to the documented statusline contract) are improvements to carry into any future build, not blockers to delivering this answer.

