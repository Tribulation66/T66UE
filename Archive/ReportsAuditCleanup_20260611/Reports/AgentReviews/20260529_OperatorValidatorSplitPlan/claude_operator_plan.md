# Operator Plan: Operator-Weighted Operator/Validator Protocol

`Operator Plan: READY`

Operator: Claude (claude-opus-4-8, read-only direct-read). Validator/Integrator: Codex.
This file is an Operator artifact (proposal), not a greenlight. Codex validates anchors and integrates.

## Context

`AGENTS.md` currently inlines the whole Operator/Validator machine in Section 3
(`Planning And Review Loop`, lines 105-205): goal clarification, folder discovery,
implementation planning, tier routing, the full Validator review loop, and the
Operator/Validator Stack. The token-weighting rule already exists (lines 182, 185:
"majority of task reasoning tokens", "roughly 70-80 percent Operator") but is written
as a *soft target with no mechanism*, so in practice the Validator (Codex) re-discovers
context and spends tokens that should have been spent by the Operator.

Goal: (1) make `AGENTS.md` a router and move the comprehensive O/V process into a
dedicated file; (2) add a hard structural mechanism that forces the Operator to do the
brunt of Tier 1 work *before* the Validator spends tokens; (3) make the token split
observable per run so weighting can be verified, not just asserted.

The core lever (the thing that actually moves spend onto the Operator): a **Packet
Completeness Gate** + **No-Rediscovery Rule**. The Validator's cheapest *legal* move
against a thin packet is to bounce it back `REVISE (incomplete packet)`, never to fill
the gap by investigating. That structurally pushes expensive discovery back to the Operator.

## Document Structure

### Stays in `AGENTS.md` (universal, needed immediately on every task)
- Section 1 Project Contract (goal derivation, `operator-state.json` read, Mini exclusion).
- Section 2 Proven Process Fidelity (separate concern; leave in place this pass).
- Tier 0/Tier 1 one-paragraph definition + the exact user-facing footer (tier line + token block, lines 137-148). These are universal output contracts.
- Accepted Process Registry, with the `Validator review` and `Operator/Validator stack` rows rewritten to point at the new protocol doc.
- A new ~8-line **Operator/Validator routing stub** replacing the bulk of Section 3: "For any Tier 1 request, the Operator-first protocol in `OPERATOR_VALIDATOR_PROTOCOL.md` is mandatory" + the non-negotiable invariants (Operator packet before Validator tokens; packet completeness gate; strict `Verdict:` first line; no `ANTHROPIC_API_KEY`).

### Moves to the new file
- Goal Clarification, Folder Instruction Discovery, Implementation Planning detail.
- Full Validator Review loop (verdict routing, fallback reviewer, greenlight rules).
- Operator/Validator Stack detail (roles, token split, tool profiles, model pin, helper invocations).
- Delegation and Error Handling.
- NEW: Operator Packet contract + Completeness Gate, No-Rediscovery rule, Validator escalation triggers, Token-Routing metadata + per-run ledger.

### Location
**`C:\UE\T66\OPERATOR_VALIDATOR_PROTOCOL.md`** — single root-level file adjacent to `AGENTS.md`, maximally discoverable, no new folder (avoids overbuild). If more process docs get extracted later (e.g. PPF), promote to a `Process/` folder with `PROCESS_AGENTS.md` router; deferred now.

## Process Rules That Force Operator-First Spend

1. **Operator-first discovery.** On a Tier 1 task, no Validator token may be spent on independent repo discovery until a complete Operator Packet exists.
2. **Packet Completeness Gate (first Validator action).** Codex's first step is a cheap structural check: all required packet sections present and non-empty, every load-bearing claim carries a concrete `path:line`/asset anchor, verification plan present with expected pass markers. If incomplete -> emit `Verdict: REVISE` enumerating *all* missing fields in one pass. Codex does **not** fill gaps itself.
3. **No-Rediscovery Rule.** The Validator spot-checks cited anchors; it does not re-map the codebase the Operator already mapped. Broad re-exploration is allowed only under the escalation triggers below.
4. **Operator owns the expensive verbs** (already AGENTS:182): reading owning instructions, mapping seams, resolving ambiguity into safe assumptions, naming exact files/assets, designing the patch, naming verification gates.
5. **Redesign-is-a-bounce.** If the Integrator finds itself redesigning rather than applying, that is evidence the packet was incomplete -> `REVISE`, not silent rework.

## Operator Packet Contract (Tier 1)

Saved as `Reports/AgentReviews/<TaskSlug>/plan_packet.md` (extends today's informal format). Required sections:

1. Working Goal (one sentence) + Tier.
2. Roles + helper/profile used + Operator run dir / manifest path.
3. User Constraints + Out-of-scope (incl. Mini exclusion).
4. Applicable Instructions Read (each `AGENTS`/process doc + one-line takeaway).
5. **Evidence / Live Findings** — minimum bar: every proposed change cites >=1 `path:line` anchor; every named risk cites the code that creates it.
6. PPF / Artifact Parity / Mechanism gates when process-governed.
7. **Proposed Patch Approach** — exact files, the change per file, why, rollback note.
8. **Verification Plan** — exact commands/captures + expected pass markers.
9. **Token-Routing Metadata** (new block, see below).
10. Operator Position + open decisions for the Validator.
11. Anti-lookalike discriminator when process-governed.

## Validator Check Contract

Saved as `claude_review_passN.md` / Codex validation. Required:
- Strict first line `Verdict: APPROVE|REVISE|NEEDS_HUMAN_DECISION|BLOCK` (existing parser contract in `Invoke-ClaudeDirectRead.ps1` / `Invoke-CodexPlanReview.ps1`).
- Completeness Gate result (PASS, or FAIL + missing fields).
- Anchor spot-check table: each verified anchor -> confirm/contradict.
- Scope + instruction-compliance check.
- Findings by severity (Blocker/Major/Minor).
- Missing verification.
- **Validation depth used** (targeted | deepened) + token spend.

## When the Validator May / Must Deepen

- **May deepen** (discretion): a cited anchor does not resolve or contradicts its claim; a load-bearing claim has no anchor; PPF method substitution is involved.
- **Must deepen** (required): packet passes completeness but the change touches a destructive/irreversible, safety, credential, build, or release surface; or the verification plan cannot prove the goal; or live state contradicts a Blocker-class claim.
- **Otherwise**: stay targeted (the default; cheapest path).

## Codex Behavior When Claude Is Operator But Only Codex Can Edit

- Claude produces the read-only Operator Packet = proposal, not greenlight (AGENTS:158).
- Codex = Validator + Integrator: runs the completeness gate, targeted anchor validation, issues the verdict.
- On `APPROVE`: Codex applies edits **as specified** in Proposed Patch Approach, with only the minimum extra reasoning to execute safely. Material deviation -> re-validate or record a noted deviation; never silent redesign.
- Codex runs the Verification Plan, captures evidence, writes `completion_packet.md`, produces the user-facing answer + footer.
- Codex retains goal / integration / final-report ownership (AGENTS:194). Apply/integration tokens count on the Validator/Integrator side — the weighting lever is packet completeness, not relabeling integration as "operator".

## Token Accounting Per Run

- **Claude manifest fields** (exist in `Invoke-ClaudeDirectRead.ps1` manifest.json): `ClaudeTokensSpent`, `ClaudeUsage`, `ClaudeModelUsage`. Operator quotes `ClaudeTokensSpent` + run dir in packet section 9.
- **Token-Routing Metadata block** (packet section 9): `OperatorModel`, `OperatorTokensSpent`, `OperatorRunDir`, `ExpectedValidatorDepth: targeted|deepened`, `ValidatorBudgetHint`.
- **Codex goal tokens**: the existing `Codex Token Spent` footer value.
- **Per-run ledger** (new): a `## Token Ledger` section in `completion_packet.md` recording `OperatorTokens`, `ValidatorTokens`, computed `Operator share %`, and met/missed vs the ~70% target. This makes the split observable — the user's core ask.
- **User-facing footer**: keep the exact existing shape (`AGENTS.md` 138-148) unchanged so the parser/contract is stable; add the Operator-share line in the completion packet (optionally echo in chat only if the user wants it).

## Implementation Phases

- **Phase 1 (smallest, doc-only, highest leverage).** Create `OPERATOR_VALIDATOR_PROTOCOL.md` with the moved O/V machinery + the new Operator Packet contract, Completeness Gate, No-Rediscovery rule, escalation triggers, Token-Routing block. Slim `AGENTS.md` Section 3 to the routing stub; keep Tier defs, footer, registry rows; repoint the two registry rows. No script changes — behavior already improves because the gate is now explicit and enforceable.
- **Phase 2.** Add packet/validator templates (appendix in the protocol doc or `Reports/AgentReviews/_templates/`); add the `## Token Ledger` convention to completion packets.
- **Phase 3 (deferred).** Helper automation to compute/record Operator share; quota router (already deferred AGENTS:205).

## Risks / Failure Modes / Token-Waste Rules

- **Over-ceremony on simple tasks** — packet contract is Tier 1 only; Tier 0 stays light (mirrors AGENTS:133).
- **REVISE ping-pong** — completeness is a fixed checklist; Codex must enumerate *all* missing fields in one pass, not drip-feed.
- **Validator under-checking** — mitigated by mandatory escalation triggers.
- **Doc drift / two sources of truth** — `AGENTS.md` keeps only the pointer; the protocol doc is sole authority; registry rows point to it.
- **Operator can't run verification** (read-only Claude can't build/capture) — the Verification Plan is a spec Codex executes; do not require the Operator to have run it.
- **Accounting gaming** — counting integration tokens as "operator" would defeat the purpose; keep apply on the Validator side, lever stays packet completeness.

## Critical Files

- `C:\UE\T66\OPERATOR_VALIDATOR_PROTOCOL.md` (new, authoritative).
- `C:\UE\T66\AGENTS.md` (slim Section 3 -> router stub; repoint registry rows; keep Tier defs + footer).
- Packets continue under `C:\UE\T66\Reports\AgentReviews\<TaskSlug>\` (`plan_packet.md`, `claude_review_passN.md`, `completion_packet.md`).
- Helpers unchanged in Phase 1: `Scripts\Invoke-ClaudeDirectRead.ps1`, `Scripts\Invoke-ClaudePlanReview.ps1`, `Scripts\Invoke-CodexPlanReview.ps1`, `Scripts\Set-T66Operator.ps1`.

## Example Templates (condensed)

### Operator packet token block (section 9)
```text
TOKEN ROUTING
OperatorModel: claude-opus-4-8
OperatorTokensSpent: <from manifest ClaudeTokensSpent>
OperatorRunDir: Reports/AgentReviews/ClaudeDirectRead/<stamp>-<task>-Operator
ExpectedValidatorDepth: targeted | deepened
ValidatorBudgetHint: <one line: what to spot-check>
```

### Completion packet ledger
```text
## Token Ledger
OperatorTokens: <n>
ValidatorTokens: <n>
OperatorShare: <pct>%  (target >= ~70%)
Met: YES/NO
```

### AGENTS.md routing stub (replaces bulk of Section 3)
```text
## 3. Operator/Validator Routing
- For any Tier 1 request, OPERATOR_VALIDATOR_PROTOCOL.md is mandatory.
- Invariants: Operator packet complete BEFORE any Validator discovery token;
  Validator's first move is the Completeness Gate (REVISE if incomplete, never fill gaps);
  strict `Verdict:` first line; no ANTHROPIC_API_KEY; Operator artifacts are proposals, not greenlights.
- Tier definitions and the user-facing token/tier footer remain in this file.
```

## Verification (for Codex after integration)

1. Confirm `OPERATOR_VALIDATOR_PROTOCOL.md` contains every moved subsection; no orphaned references in `AGENTS.md`.
2. Grep the repo for links to AGENTS Section 3 anchors and repoint to the new doc.
3. Confirm the two Accepted Process Registry rows point at the new doc.
4. Confirm `Invoke-ClaudeDirectRead.ps1` verdict parser still matches the documented `Verdict:` contract (no script change needed).
5. Dry-run one Tier 1 packet against the new completeness checklist to confirm it is enumerable and not ambiguous.
6. No build/cook needed (docs-only); report verification skipped for runtime per AGENTS rule.
