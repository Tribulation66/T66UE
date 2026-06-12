`Operator Assessment: NEEDS_FIX`

Read-only inspection of all four files complete. The bulk of the request is satisfied; one substantive wiring gap and two cosmetic items hold it back from a clean PASS.

## What passes

| Requirement | Status | Evidence |
|---|---|---|
| Stronger Operator vs Validator rules | PASS | `AGENTS.md:181-185` defines Operator work (expensive first-pass) vs Validator work (independent critique, not a second implementation), plus the 70-80/20-30 token split. |
| Claude read-only yet heavy Operator | PASS | `AGENTS.md:186` "Editing permission is separate from Operator status. Claude can be the heavy Operator while remaining read-only." Reinforced by read-only baseline profile `AGENTS.md:198`. |
| Token report format (Codex → number, Claude → number, tier) | PASS (with cosmetic flag) | `AGENTS.md:140-148` emits the exact shape ending in `**Tier: Tier N**`. |
| Widget text significantly larger | PASS (build/visual unverified) | `MainWindow.xaml` uses FontSize 26-28 across all rows, row height 54, window 208. Fits the box math (3×54 + 16 margin = 178 < ~188 usable). |
| Only the percentage is badged | PASS | Only `ClaudePercentBadge`/`CodexPercentBadge` wrap `ClaudeText`/`CodexText` (`xaml:62-78, 93-109`). Operator row value sits in the neutral row border, not a colored badge. |
| Badge color: red low / yellow mid / green high remaining | PASS | `MainWindow.xaml.cs:173-192` `GetRemainingBrush`: ≤25 → red `0xB91C1C`, ≤50 → amber `0xA16207`, else green `0x166534`, keyed on `WeeklyRemainingPercent`. Direction is correct (low remaining = red). |
| JSON usage captured + surfaced in manifest | PARTIAL — see finding | Capture is implemented (`Get-ClaudeTokenTotal`, manifest `ClaudeTokensSpent`/`ClaudeUsage`/`ClaudeModelUsage`/`ClaudeJsonPath`, lines 318-374, 639-641) but gated behind json mode. |

## Blocking-ish finding (the NEEDS_FIX driver)

**Token capture is not wired into the documented token-report flow.** `Invoke-ClaudeDirectRead.ps1:39-40` defaults `-ClaudeOutputFormat "text"`. In text mode `Export-ClaudeStdoutArtifact` returns `$null` (line 295), so `ClaudeTokensSpent`, `ClaudeUsage`, `ClaudeModelUsage`, and `ClaudeJsonPath` are all null/empty in the manifest. The capability exists, but it only fires when the operator explicitly passes `-ClaudeOutputFormat json`.

Nothing in `AGENTS.md` ties the "Claude Tokens Spent" report line (`AGENTS.md:144-145`) or the Claude direct-read process row (`AGENTS.md:226`) to running the helper in json mode. Net effect: in normal use the report will read `Claude Tokens Spent: Unavailable` even though the plumbing works. The `Unavailable` escape hatch makes this non-fatal, but it defeats the stated goal of surfacing Claude usage.

**Recommended fix (pick one):**
- Default `-ClaudeOutputFormat` to `json` for Operator mode (text remains opt-out), **or**
- Add an explicit instruction in `AGENTS.md` §Operator/Validator Stack and the Accepted Process Registry "Claude direct-read" row that the helper must be run with `-ClaudeOutputFormat json` whenever Claude token spend must appear in the report, and have the manifest's `ClaudeJsonPath`/`ClaudeTokensSpent` cited as the token-report source.

## Minor items (non-blocking)

1. **Label inconsistency** `AGENTS.md:141` `Codex Token Spent:` (singular) vs `:144` `Claude Tokens Spent:` (plural). It matches your request verbatim, so no change is required — but if you want symmetry, normalize both to `Tokens`.
2. **"Yellow" is amber** `MediumRemainingBrush 0xA16207` reads as dark amber/orange rather than a bright yellow. Functionally correct for the mid band; bump toward a brighter hue only if the at-a-glance read matters.

## Missing verification (for Codex to close)
- No build/run proof the tray renders at FontSize 28 within Width=400/Height=208 — box math fits but confirm no clipping on the live widget, including a long operator name (`OperatorText` uses `CharacterEllipsis`, so verify it truncates rather than overflows).
- No artifact showing a real json-mode direct-read run with non-null `ClaudeTokensSpent` in `manifest.json`. Capture one to prove the end-to-end token-report path before treating req #7 as done.

Handoff: this is an operator artifact, not a greenlight. Codex should validate the two recommended fixes against live state and produce the json-mode manifest sample.

**Tier: Tier 1**
