Verdict: NEEDS_HUMAN_DECISION

## Blockers
- None. The packet does not request a live-repo change; the live per-mob renderer is explicitly left untouched. Nothing here is unsafe to "proceed" with because the operative decision is to *not* land.

## Major Issues
- The packet is a closeout report, not an implementation plan. There is no actionable next-step scope that Codex can own without a user direction choice. The "Proposed Next Scope" lists four mutually exclusive architectural directions, at least one of which (#3, reducing transform update frequency / material interpolation) is explicitly a gameplay-visual tradeoff requiring user sign-off. Codex cannot pick among these on its own.
- All performance candidates regressed against the `189.65` median baseline or crashed. The conclusion (don't land) follows correctly from the evidence, so the de-risk outcome is sound — but it means the working goal (replace per-actor with instanced rendering) is unmet and the path forward is a product/architecture decision, not a Codex revision.

## Minor Issues
- The Stage 1 / Full Rollout proof tables share the same binary hash (`523293239A13...`) as a rejected performance candidate. This is internally consistent (the correctness build is the sparse-custom-data build that "regressed vs before"), but the packet labels the same artifact "Pass" in proof tables and "Correct but not acceptable" in measurement. Worth a one-line note that "Pass" means correctness-only, not performance acceptance.
- The `1/1` probe rows are clearly flagged as rejection-only probes, which is good discipline. No issue, just confirming the asymmetric acceptance standard (full 3/3 only to *accept*, single probe to *reject below baseline*) is reasonable here.

## Clarifying Questions
- Which of the four B.13R directions does the user want pursued, if any? #1 (instance only static/death/pooled states), #2 (ISM with proven median win), #3 (gameplay-visual tradeoff), #4 (GPU/VAT manager-owned positions) differ substantially in scope and risk.
- Is the user comfortable closing B.13 as "not landed" and opening B.13R as a fresh reviewed plan, rather than continuing to tune HISM?

## Required Verification
- None additional for the closeout itself. The de-risk verification is thorough: engine-source audit of `BatchUpdateInstancesTransformsInternal` / `BuildTreeIfOutdated` correctly explains why `bAutoRebuildTreeOnInstanceChanges=false` does not eliminate per-frame cost; baselines used stable hashes, fixed resolution, fixed `HeroHPOverride`, same saturation contract. Root-cause reasoning (moving instances defeat instancing's draw-call savings) is supported by the candidate matrix.
- Before any future B.13R lands: a candidate must prove a median FPS win over `189.65` at full resolution under the same `enemywaveperf` contract, not just 1%/0.1% low improvements.

## Rationale
The reviewed work is a correctly-conducted negative result: every dynamic-instancing path regressed or crashed, and the decision to keep the live renderer unchanged is the safe, evidence-backed call. There is nothing unsafe to approve and nothing for Codex to revise. The only open item is forward direction, and the proposed next-scope candidates hinge on product/architecture/gameplay-tradeoff choices that only the user can make — hence NEEDS_HUMAN_DECISION. Codex should record the decision block, surface the four B.13R directions, and stop until the user picks one.

