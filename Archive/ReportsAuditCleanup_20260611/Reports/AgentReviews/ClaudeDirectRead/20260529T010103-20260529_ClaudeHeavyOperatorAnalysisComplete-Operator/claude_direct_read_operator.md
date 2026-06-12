## Operator / Validator

Operator = the model that does heavy planning, investigation, proposal/implementation design, and (when approved) writes. Validator = the model that checks assumptions, scope, verification, and final claims. Today: Claude is read-only Operator (`claude-opus-4-8`, high, plan mode, `Read/Grep/Glob`); Codex retains goal tracking, repo integration, edit application, final verification, and the user-facing completion report.

## Recommended Split

Target **Claude 70–80% Operator / Codex 20–30% Validator-Integrator by token volume**, but stop measuring it as a single ratio. Track two separate ratios (analysis tokens vs. integration tokens) — see Metrics. Your 80/20 instinct is directionally right but rests on a wrong premise about *which activity* burns tokens.

## Heavy Token Drivers

In agentic coding the cost is overwhelmingly **input/context tokens, not output**:

1. **Context ingestion** — reading large/many files (`.uasset` adjacents, CSV/JSON data tables, the dense Markdown process docs in `Gameplay/Combat`), repeated re-reads, and exploration sweeps. This is the single biggest line item.
2. **Reasoning over that context** — multi-step investigation, cross-file consistency checks, proposal generation. High-effort planning amplifies this.
3. **Round-trip duplication** — when one model investigates and the *other* must re-read the same files to act or verify. Every handoff that forces re-ingestion roughly doubles the context cost for that slice of work.
4. **Output/diffs** — comparatively cheap. A 200-line edit is a rounding error next to reading the files needed to write it.

The implication that drives everything below: **token share is won by owning reading/investigation/planning, not by owning edits.** Edits are token-light.

## Is 80/20 Right?

Partly. The flaw: 80/20 assumes validation is cheap. Done properly, validation requires re-reading much of the same context to check assumptions and verify claims — so a *rigorous* validator can easily consume 30–40%, not 20%. Two correctives:

- If you want Claude at 70–80%, the lever is **giving Claude the investigation + planning + proposal load**, which it already largely has as a reader. Edit rights barely move the needle.
- The real waste isn't the split percentage — it's **handoff re-reads** (driver #3). Optimizing the split while ignoring duplication can *raise* total tokens. Minimize re-ingestion first; the ratio follows.

So: 80/20 is a fine *aspiration* for who-does-the-thinking, but a poor *model* of cost. Reframe as "Claude owns analysis context; Codex validates against a focused subset, not a full re-read."

## Current Gap

Read-only Claude can **already** plausibly reach 70–80% of useful workload, because the dominant cost (reading + reasoning + proposal authoring) is fully inside its current `Read/Grep/Glob` profile. The gap is **not capability — it's leakage**:

- Codex re-reads files Claude already digested to apply edits → duplicated context cost shifts share back to Codex and inflates totals.
- Claude's proposals may be under-specified (prose, not patch-ready), forcing Codex to re-derive → more re-reading.
- No measurement separating "analysis tokens" from "integration tokens," so the split is unprovable today.

The gap is process/output-format, not a missing edit tool.

## Recommended Operating Model

Smallest safe change, in order of leverage:

1. **Make Claude's proposals patch-ready.** Operator output should be a precise change spec: exact file paths, anchored before/after snippets, and rationale — enough that Codex applies mechanically without re-investigating. This alone captures most of the 70–80% goal with **zero new tools**.
2. **Front-load context once.** Claude does the deep read; Codex validates against the *specific anchors* Claude cites, not a fresh full sweep. Kills the duplication driver.
3. **Keep Claude read-only by default.** Do not grant edit rights to chase token share — that's solving the wrong problem.

This requires no change to Claude's tool profile and no new permissions.

## Direct Edit Policy

**Default: no — Claude should not directly edit.** Edits are token-cheap, so granting `Edit`/`Write` does little for the 70–80% target while adding integration and safety risk (especially around `.uasset`, data tables, and editor state). Token share is earned by analysis, not authorship.

**Bounded exception, only if measurement proves handoff re-reads are large:** allow Claude to edit a narrow, low-risk class under a task-specific reviewed plan that names the exact profile:
- Scope: plain-text source/config/Markdown only — explicitly **never** `.uasset`, no production asset writes, no Unreal Python, no editor automation, no unrestricted `Bash`, no `bypassPermissions`.
- Tooling: add `Edit` (prefer over `Write`) to a named profile; Codex still applies the commit and runs verification.
- Trigger: user-approved, per-task, with the file class enumerated.

Even then, the justification is "eliminate re-read duplication on text files," not "increase Claude's percentage."

## Codex Validation Role

Codex remains Validator-Integrator and should keep:
- **Goal tracking** and user-facing completion report (single source of truth for "done").
- **Repo integration / commits** — Claude proposes, Codex commits.
- **Final verification** — builds, data-table integrity, asset references, anything requiring editor/Unreal Python or shell, which Claude is barred from.
- **Assumption/scope audit** against Claude's cited anchors — targeted, not a full re-read.

Codex stays the gatekeeper for everything risky or irreversible (asset writes, editor automation, shell).

## Metrics

Prove the split **without quota-denominator inference** by measuring observable, model-attributable counts per task:

1. **Analysis tokens (Claude):** input+output tokens Claude spends on read/investigate/propose.
2. **Integration tokens (Codex):** tokens Codex spends applying + verifying.
3. **Operator share = Claude analysis ÷ (Claude analysis + Codex integration).** Target 0.70–0.80.
4. **Re-read duplication ratio:** fraction of files Codex reads that Claude already read in the same task. Target → near 0. This is the real efficiency metric.
5. **Proposal-acceptance rate:** % of Claude proposals Codex applies without re-investigation. Rising rate = proposals are patch-ready.
6. **Handoffs per task:** count of Operator↔Validator round-trips. Fewer = less duplication.

Log per task; no global quota denominator required — all are direct counts and ratios.

## Final Recommendation

Your 80/20 instinct points the right way, but the lever you're reaching for (edit rights) isn't where the tokens are. Heavy usage lives in **reading and reasoning**, which read-only Claude already owns. To make Claude a true 70–80% Operator: (1) ship **patch-ready proposals**, (2) **front-load context once** and have Codex validate against cited anchors to kill re-read duplication, and (3) **keep Claude read-only**, granting a narrow text-only `Edit` profile *only if* metric #4 proves duplication is costly — never for `.uasset`/editor/Unreal/shell. Codex stays Integrator and final verifier. Measure analysis-vs-integration tokens and re-read duplication, not a single blended ratio.

