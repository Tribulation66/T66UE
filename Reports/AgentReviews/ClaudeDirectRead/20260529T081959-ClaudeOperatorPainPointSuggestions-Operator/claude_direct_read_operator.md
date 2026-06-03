I've read all four files. Here's my prioritized recommendation set. These are suggestions for Codex to validate — no files touched.

---

## Context grounding

- The helper (`Invoke-ClaudeDirectRead.ps1`) leaves session persistence **on** by default (line 44, 553) and uses `--resume` for max-turn auto-continuation (lines 682–714). Resume is what replays prior `thinking`/`redacted_thinking` blocks back to the API.
- FullOperator already defaults to **uncapped** `MaxTurns=0` (line 39, 561) and **unbounded** timeout (lines 750–752), so the max-turn auto-continue path rarely fires for normal Operator runs.
- Read-only/packet runs (this profile, and `Invoke-ClaudeReadOnlyOperator.ps1`) **cannot** build, run commandlets, capture, or judge visuals — by design (header lines 859–865). FullOperator **can** (lines 844–858).
- Evidence anchoring (OVP §5, lines 287–297), diagnostic visibility check (AGENTS.md line 200), and anti-lookalike discriminator (OVP §11) already exist as gates.

---

## Recommendations (prioritized by wasted-work reduction)

### P1 — Stop the 118K-token resume failure (Pain #1)

- **Problem:** A persisted-session `--resume` replays modified `thinking`/`redacted_thinking` blocks; the API rejects them, the run dies with zero accepted artifact.
- **Suggested change:** Do **not** make `-NoSessionPersistence` a blanket global default. Instead, make persistence **conditional**: default it **off** whenever `MaxTurns` is `0` (uncapped) — which is the FullOperator baseline — since the only thing persistence buys is the `--resume` max-turn auto-continue, and that loop is unreachable when turns are uncapped (lines 682–714 only resume on `error_max_turns`). Keep persistence **on** only when the caller explicitly sets `-MaxTurns N`, where the auto-continue safety net actually has a job. Additionally, add the thinking-block error string to `Test-ClaudeUnavailableSignal`'s sibling logic so a detected resume/thinking failure auto-retries the attempt **fresh** (persistence off) instead of burning the attempt budget.
- **Why it helps:** Kills the exact failure for the common case (uncapped Operator runs) at zero feature cost, while preserving the continuation feature for the rare timeboxed case that needs it.
- **Risk/tradeoff:** Explicitly-capped runs that hit the turn ceiling lose auto-continue when persistence is off — but those are opt-in probes, and the operator can re-invoke. A fresh-retry-on-thinking-error could mask an underlying CLI bug; log the `FailureKind` so it stays visible.
- **Already partly solved?** Partly — the flag and the failure manifest exist (`FailureKind`, `MaxTurnContinuationAttempted`). What's missing is the **conditional default** and a **thinking-block-specific signature** so the helper self-heals instead of returning a dead run.

### P2 — Route proof-bearing tasks to FullOperator from the start (Pains #3 and #5 together)

- **Problem:** Claude returned implementation *packets*, then Codex had to apply, build, run commandlets, bind, capture, and judge — and some cited evidence paths weren't authoritative until Codex reran with explicit paths. Both stem from the same root: a **read-only/packet** run was used for a task whose acceptance requires **produced** evidence.
- **Suggested change:** Add a routing rule (AGENTS.md "Request Routing" or OVP flow step 4): if a task's acceptance gate requires a build log, commandlet marker, capture (PNG/MP4), or visual judgment, it must be approved and run as **FullOperator** (with the Codex approval artifact), not as a read-only packet whose deliverable is a proposal. Pair this with an **evidence-provenance tag** in the packet's §5: each anchor labeled `produced-this-run` (with the exact output path the run wrote) vs `pre-existing-reference`. A proof-class anchor that is only `pre-existing-reference` does not satisfy a current-verification gate.
- **Why it helps:** Moves the build/commandlet/capture spend onto the Operator (the intended 70–80% split, OVP lines 16–22) instead of leaving it for Codex, and makes weak/stale evidence visible at packet time rather than after Codex reruns.
- **Risk/tradeoff:** FullOperator needs the Codex approval artifact up front, so a misclassified task adds one approval round-trip. Mitigate by classifying at task-contract time. Provenance tagging adds packet ceremony — keep it to proof-class anchors only, not every `path:line`.
- **Already partly solved?** Yes, structurally — FullOperator exists, unbounded timeout supports long capture work, and §5 already demands anchors. The gap is (a) a **rule that forces** proof tasks onto FullOperator instead of packets, and (b) **freshness/authoritativeness labeling** of anchors.

### P3 — Add a units/bounds parity check for visual tasks (Pain #4)

- **Problem:** Bounce visual was too small/misaligned vs the hit volume; root cause was normalized mesh bounds vs centimeter gameplay bounds. Codex caught it; Claude's early proof showed runtime/damage worked but didn't test scale.
- **Suggested change:** Extend the existing Artifact Parity Gate / Mechanism Manifest (AGENTS.md §2) with an explicit **scale/units parity line** for any visual whose silhouette must register against a gameplay volume: declare the source unit space (normalized mesh, UV, world-cm), the target volume's units, and the expected on-screen size relative to the hit/collision volume — then require that the discriminator capture frame the **VFX silhouette against the gameplay volume**, not the VFX alone.
- **Why it helps:** "Runtime works" and "looks like the effect" are both passable while scale is wrong; an explicit units line plus a side-by-side framing is the discriminator that separates the cheap-wrong-result (correctly-typed but mis-scaled effect) from the intended one.
- **Risk/tradeoff:** Adds a gate line to every visual packet; for effects with no gameplay-volume relationship it's `N/A` and shouldn't become noise.
- **Already partly solved?** Partly — the diagnostic visibility check (AGENTS.md line 200) names "target size" and the anti-lookalike rule exists, but neither calls out **unit-space mismatch vs a reference volume** as a required, named check.

### P4 — Structured blocker-triage handoff (Pain #2)

- **Problem:** Claude correctly stopped at unrelated compile blockers (protocol-safe), but Codex had to inspect the blocker and make a narrow integration fix to proceed.
- **Suggested change:** When an Operator stops on an out-of-scope blocker, require a structured **blocker handoff** in the artifact: the failing signature, the minimal suspected integration point (`path:line`), whether it's in-scope-adjacent, and a `pending_issues_<folder>.md` entry. This is a small addition to the existing `BLOCK` verdict / Pending Issues flow, not a new mechanism.
- **Why it helps:** Codex spends its turn *deciding* on a pre-diagnosed blocker rather than *rediscovering* it — keeps the validator-not-author boundary while cutting Codex's discovery cost.
- **Risk/tradeoff:** A read-only Operator's blocker diagnosis is unverified (it can't compile), so the handoff must be labeled `suspected`, not authoritative — otherwise it risks becoming a stale fix recipe. Low risk overall.
- **Already partly solved?** Mostly yes — `BLOCK` verdict (OVP line 477), Pending Issues (AGENTS.md 187–190), and the "stop on out-of-scope" rule already cover the *behavior*. The only gap is a **consistent triage shape** so the handoff is reliably actionable.

---

## Net

The single highest-value change is **P1** (direct, recurring token burn, cheap conditional fix). **P2** is the structural one — it reassigns proof work to the Operator and is where most of the repeated Codex effort in #3/#5 actually lives. **P3** and **P4** are targeted gate refinements to checks that already exist. None require a broad rewrite; all are additive rules or a scoped helper-default change, and all keep Codex as validator/final-proof owner.

**Codex Token Spent:** 0

**Claude Tokens Spent:** Unavailable (this read-only operator profile does not expose its own count to me; read the run manifest `ClaudeTokensSpent` for the authoritative figure)
