# Content Audit Full - Validator Prompt

Working task:
Operator: Codex
Validator: Claude
Scope: Produce the full source-grounded CONTENT audit document for T66/Chadpocalypse from a player/design perspective, including main game and Mini/minigame-class modules (`T66Mini`, `T66TD`, `T66Idle`, `T66Deck`) at full depth.
Stop condition: completed document plus validation summary. Descriptive only: flag suspected problems, do not fix. No git operations.

Original user request:

CONTENT AUDIT - GENERATE THE FULL DOCUMENT. Critique accepted. Produce the full CONTENT audit document: ground-truth description of what the game ACTUALLY IS from a player/design perspective, source-grounded, flagging what's broken/hidden/deprecated. Generate and answer the full question set from the repo. Scope change: Mini/minigames are now IN SCOPE at FULL DEPTH. Audit T66Mini and minigame-class modules T66TD, T66Idle, T66Deck as thoroughly as main game. Shared schema mandatory: lifecycle tags ACTIVE, DEMO_GATED, HIDDEN_RUNTIME, PARTIAL, DEPRECATED, COMPAT_LEGACY, BROKEN, STUB, ORPHAN_SUSPECT, UNKNOWN; evidence tier on every claim READ / STATIC_TRACE / PRIOR_ARTIFACT / RUNTIME_VERIFIED; element IDs CONTENT-{AREA}-{NNN}; finding IDs CONTENTFIND-{NNN}; include content-specific visibility route. Stale-comment caution: code comments/docs may lag runtime. Verify against current code/runtime behavior before tagging, and flag doc-drift separately. Document structure: scope/date/release-demo assumptions; player walkthrough; content surface map by domain; element cards; hidden/deprecated/partial register; verification backlog; cross-reference appendix. Core goal: distinguish ACTIVE vs DEMO_GATED vs HIDDEN_RUNTIME vs DEPRECATED. Cite final counts. Run mismatch hunt. Use deployable sub-agents. Operator Codex, Validator Claude Code. Cite file:line throughout. No fixes, no git operations.

Relevant repo rules:
- Root `AGENTS.md` is authoritative for T66 process.
- `.t66/operator-state.json` says Codex Operator and Claude Validator.
- `OPERATOR_VALIDATOR_PROTOCOL.md` requires Claude independent answer first and cross-review later through `Scripts\Invoke-ClaudePlanReview.ps1`.
- `ANTHROPIC_API_KEY` was checked in Process/User/Machine scope and was not set.
- `Reports/AGENTS.md` routes review packets under `Reports/AgentReviews`; `Audit/AUDIT_AGENTS.md` routes user-requested audit documents under `Audit/Pending` unless closed/approved.
- Mini/minigame default exclusion does not apply because the user explicitly opted Mini/minigames in.
- No production source/content fixes. No git operations.

Validator ask:
Produce an independent, repo-grounded content-audit answer plan and high-value findings for the final document. Inspect current repo state read-only. Pay special attention to lifecycle tag distinctions, stale docs/comments, Mini/minigame reachability, and mismatch-hunt findings.
