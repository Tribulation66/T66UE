Verdict: APPROVE

Blockers
- None.

Major Issues
- None. The packet correctly identifies root `AGENTS.md` as the canonical commit/push policy and frames release/Steam/demo docs as adjacent constraints rather than overrides.

Minor Issues
- The planned answer lists `Release/README.md` only in evidence, not in the answer summary; consider one line acknowledging it as the entry point into the release doc set so the user can navigate.
- "Generic Git policy" phrasing is slightly soft. The root rules are specific (tag naming, `ProjectVersion` bump, completion criteria, no blanket reset/clean). Calling them "generic" risks understating them; "root-level Git policy" reads cleaner.
- Recommend explicitly stating in the answer that `Release/RELEASE_AGENTS.md` does not redefine commit/push mechanics — it routes versioning/tag/upload work — so the user does not infer it supersedes root rules.

Clarifying Questions
- None for Codex. The packet is read-only and the user's question is bounded.

Required Verification
- None beyond what the packet already performed (targeted `rg` + direct line reads). No runtime, build, or staged validation is appropriate for a read-only documentation question.
- When delivering the answer, cite file paths with line ranges as the packet already does, so the user can verify directly.

Rationale
- Scope is correct: the user asked "what are the instructions" and "anywhere else," and the packet covers root `AGENTS.md` plus every adjacent router that touches commit/push, release, upload, or commit-exclusion (secrets, vendor files). Agent-router search was performed across `AGENTS.md` and `*_AGENTS.md`, which is the right discovery surface per the root router's folder-instruction discovery rule.
- The known-risks section correctly preempts two real failure modes: (1) the Steamworks doc's stale 2026-04-24 snapshot being mistaken for current Git state, and (2) conflating Steam upload with `git push`. Both are exactly the kind of cross-doc confusion that would mislead the user.
- No PPF is needed (no runtime/visual artifact), no Plan agent is needed (no implementation), and no human decision is pending — this is purely a documentation lookup answer. Safe for Codex to deliver the answer as planned.

