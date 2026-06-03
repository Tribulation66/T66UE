The findings are written pending your approval. Summary of what I found:

**Yes — Codex token spend is available without the goal tool.** The reliable non-goal source is the **session rollout JSONL** under `C:\Users\DoPra\.codex\sessions\YYYY\MM\DD\rollout-*-<session-uuid>.jsonl`. Every model turn appends a `token_count` event:

- `total_token_usage.total_tokens` — cumulative session spend
- `last_token_usage.total_tokens` — the current turn's delta
- plus `model_context_window`, all timestamped

Verified on a real session: 14 token events, cumulative total climbing 48,854 → 1,341,327 with matching per-turn deltas.

**Source rundown:**
| Source | Status |
|---|---|
| Rollout JSONL `token_count` events | **Usable — recommended** (current-turn accurate) |
| `logs_2.sqlite` / `state_5.sqlite` | Same data, but ~1 GB, hot WAL, lock-prone — heavier alternative |
| `goals_1.sqlite` | The old goal source — excluded by design |
| `config.toml` | No token data (only confirms `CODEX_HOME`) |
| Environment variables | Not a source — only path pointers, no live counts |
| `log/codex-tui.log` | Stale (Feb 7), unusable |
| `session_index.jsonl` | No counts, but identifies the active session's rollout |

**Recommendation:** a read-only helper reads the last line of `session_index.jsonl` for the active session UUID, globs its rollout file, scans from the end for the last `token_count` line, and reports `last_token_usage` (this turn) + `total_token_usage` (session). No goal tools, no edits, no DB locking.

**Key caveat for Codex:** a turn's own `token_count` line is flushed only when that turn completes, so a final-answer footer reads spend *through the last completed turn* — that's the gap `/goal` used to paper over. Decide whether "through last completed turn" is acceptable, and which figure to label as "spent" (`total_tokens`, possibly net of `cached_input_tokens`).

The write to `claude_operator_findings.md` needs your approval to land — approve it and the full artifact saves to the review folder.
