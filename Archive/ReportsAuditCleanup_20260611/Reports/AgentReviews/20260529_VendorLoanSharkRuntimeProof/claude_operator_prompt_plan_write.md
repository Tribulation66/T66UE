Working task:
Write the plan packet for the Vendor failed-steal and Loan Shark debt-spawn staged runtime proof pass.

Operator:
Claude (`claude-opus-4-8`, FullOperator with report-only write scope)

Validator:
Codex

Scope:
You may write exactly one file:
`C:\UE\T66\Reports\AgentReviews\20260529_VendorLoanSharkRuntimeProof\operator_plan_packet.md`

Do not edit source/data/config/content/scripts/staged builds. This is still planning only.

Context:
You just completed a read-only API confirmation pass for this task. Use those findings and, if needed, re-read narrow source anchors to produce a complete plan packet. Required known findings:
- `AT66BossBase::Die()` is protected; the public Vendor kill path should use `TakeDamageFromHeroHit(int32, FName, FName)`.
- Deterministic Vendor failed steal can use `ResolveShopStealAttempt(0, false, false)` to guarantee `Miss` with zero RNG.
- `LoanShark` member is private, but `AT66GameMode::TrySpawnLoanSharkIfNeeded()` is public.

Plan packet requirements:
- Include the exact real APIs and implementation anchors for Vendor and Loan Shark.
- Include existing AutoQA registration/exit pattern anchors.
- Include proposed proof mode names and terminal summary markers.
- Include files to touch, verification commands, staged log paths, rollback considerations, non-shipping/production-neutrality approach, and risks.
- State whether any Pablo decision is needed before implementation. If no decision is needed, state implementation can proceed after Codex approval.

Output:
Write `operator_plan_packet.md` and then print a concise completion summary.
