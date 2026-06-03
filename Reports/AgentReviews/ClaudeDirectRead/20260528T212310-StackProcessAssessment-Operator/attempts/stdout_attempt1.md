I've inspected all seven named files. Here's my independent assessment of the T66 AI collaboration stack.

---

**Claude rating: 7/10**

The stack has genuinely strong safety and anti-hallucination engineering — well above what most solo setups bother with — but it's over-built for a one-developer workflow, carries real redundancy, and applies its heaviest gates uniformly instead of by task risk. The verdict parser is the standout: strict first-non-empty-line matching, fail-closed on malformed output, BOM handling, indented/quoted/heading rejection, and a dedicated self-test (`Test-ClaudeReviewVerdictParser.ps1`). The `ANTHROPIC_API_KEY` guard across Process/User/Machine scopes, the read-only `Read,Grep,Glob` plan-mode baseline, the "operator artifact ≠ greenlight" separation, the LFS-scan hazard warnings, and the "Unreal-owned capture, not desktop screenshot" evidence rule are all correct and earned. What drags it down: `AGENTS.md` is a very large document that every session pays for in context; Section 2 (PPF check + artifact parity gate + mechanism manifest + PPF close + mechanism close — five separate ceremony blocks) is heavy even when only visual tasks need it; cross-review is the *default* for every request including question-only ones; and the verdict contract is copy-pasted four times (AGENTS.md, README, and verbatim inside both review-script prompts) with `Get-ClaudeReviewVerdict` duplicated nearly identically across two scripts. The bones are good, the discipline is real, but the process tax and duplication are the difference between a 7 and a 9.

**Keep**
- Strict first-line verdict parser + fail-closed-on-malformed + the dedicated self-test. This is the core that makes cross-review trustworthy. Do not loosen it.
- `ANTHROPIC_API_KEY` billing guard and subscription-auth assertion (`loggedIn`/`authMethod=claude.ai`/`apiProvider=firstParty`).
- Read-only baseline tool profile (`Read,Grep,Glob`, `--permission-mode plan`) with the `-AllowBroaderTools` gate that throws on unexpected tools.
- "Operator artifact is not a greenlight" + manifest/`.report-run.json` retention tagging.
- LFS scan warning and "narrow path checks over `Content/`" rule — that's a concrete, repo-specific hazard.
- Unreal-owned capture requirement over desktop screenshots.

**Change**
- Make cross-review **risk-tiered, not default-on**. Question-only and trivial reads should skip external review by default; require it for code/content/script/asset/visual changes. Right now every question pays a full Claude round-trip unless the user remembers to say `skip Claude review`.
- Collapse the four copies of the verdict contract into one canonical block that the scripts read from a single file, rather than re-embedding it in each prompt string. Same for `Get-ClaudeReviewVerdict` — factor it into one shared module dot-sourced by both helpers.
- Move Section 2's five ceremony blocks into a separate `PROCESS_FIDELITY.md` that's referenced only when a visual/VFX/import task is detected, so non-visual tasks don't carry it in context.

**Remove or soften**
- The blanket "every substantive output, including trivial-task completion reports, requires a greenlight" rule. This is the single biggest latency/token drain and the place the process most exceeds the value it returns for a solo dev. Soften to: completion reports for trivial/data-row/typo/config tasks don't need a greenlight.
- The deeply nested fallback prose (Claude-unavailable → Codex fallback → but fallback still needs manual confirmation → malformed-is-not-availability-failure). It's *correct*, but it's spread across AGENTS.md §3 and the registry table in near-duplicate form. Keep the logic, state it once.

**Add**
- A short "task risk classifier" at the top of AGENTS.md (trivial / question / code-content / visual-asset) that explicitly routes which gates apply. Most of the waste comes from applying heavy gates uniformly; an explicit tier table fixes that without weakening safety where it matters.
- A one-line index/TOC at the top of AGENTS.md. At its current size, future agents need a map.

**Single highest-leverage improvement**

Tier the process by task risk. Right now the two most expensive mechanisms — mandatory cross-review greenlight and the Section 2 fidelity ceremony — are applied by default to *everything*, with opt-out. Invert that: classify each request (trivial/question vs. code-content vs. visual-asset) and apply only the gates that class needs. This cuts the most token and latency waste while *keeping* the strict gates exactly where they earn their cost (asset writes, VFX, builds). Everything else in the "Change/Remove" list is downstream of this one decision.

---

Note: you asked for both a Claude and a Codex rating. The above is mine. The `Codex rating: X/10` has to come from a separate Codex pass — I can't produce Codex's independent number for it.

One process flag worth surfacing: this assessment is itself substantive output, which under your own `AGENTS.md` §3 would normally require a review greenlight before delivery. I'm delivering it directly because the operator prompt that launched me explicitly framed this as a read-only findings artifact for Codex to integrate — but per your own rules, treat this as an unreviewed operator artifact, not a greenlit conclusion.

