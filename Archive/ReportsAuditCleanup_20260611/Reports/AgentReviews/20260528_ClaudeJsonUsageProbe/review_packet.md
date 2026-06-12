# Review Packet - Claude JSON Usage Probe

## Working Goal

Check what Claude Code CLI JSON output exposes for a minimal run, especially whether it includes token usage data that could support per-answer percentage calculations.

## User Request

The user asked to check what Claude JSON output gives us, because if tokens are present then the desired usage percentages may be simple calculations.

## Applicable Instructions

- Root `AGENTS.md` applies.
- This is a read-only tooling/process question. No Mini/minigame scope is involved.
- PPF is not applicable because there is no visual/media/import/build/artifact-authoring work.
- `Reports/AGENTS.md` routes review packets and proof outputs under `Reports/AgentReviews/`.

## Commands And Evidence

1. Billing/auth guard:
   - Checked `ANTHROPIC_API_KEY`, `CLAUDE_CODE_USE_BEDROCK`, and `CLAUDE_CODE_USE_VERTEX` in Process/User/Machine scopes.
   - All were unset.

2. Claude CLI identity:
   - `claude --version` returned `2.1.150 (Claude Code)`.
   - `Get-Command claude` resolved to `C:\Users\DoPra\.local\bin\claude.exe`.

3. Minimal JSON generation:
   - Ran `C:\Users\DoPra\.local\bin\claude.exe -p --no-session-persistence --permission-mode plan --max-turns 1 --output-format json`.
   - Prompt was `Reply with exactly: OK`.
   - Output artifact: `C:\UE\T66\Reports\AgentReviews\20260528_ClaudeJsonUsageProbe\claude_json_minimal.json`.
   - Stderr artifact: `C:\UE\T66\Reports\AgentReviews\20260528_ClaudeJsonUsageProbe\claude_json_minimal.stderr.txt`.
   - Exit code: `0`; stderr length: `0`.

4. Top-level JSON fields observed:
   - `type`, `subtype`, `is_error`, `api_error_status`.
   - `duration_ms`, `duration_api_ms`, `ttft_ms`, `num_turns`.
   - `result`, `stop_reason`, `session_id`, `uuid`, `terminal_reason`, `fast_mode_state`.
   - `total_cost_usd`.
   - `usage`.
   - `modelUsage`.
   - `permission_denials`.

5. Top-level `usage` values from this run:
   - `input_tokens`: `6`.
   - `cache_creation_input_tokens`: `11073`.
   - `cache_read_input_tokens`: `21351`.
   - `output_tokens`: `6`.
   - `service_tier`: `standard`.
   - `speed`: `standard`.
   - `server_tool_use.web_search_requests`: `0`.
   - `server_tool_use.web_fetch_requests`: `0`.
   - `cache_creation.ephemeral_1h_input_tokens`: `11073`.
   - `cache_creation.ephemeral_5m_input_tokens`: `0`.
   - `iterations` contained a message iteration with the same main-model token buckets.

6. `modelUsage` values from this run:
   - `claude-haiku-4-5-20251001`: input `441`, output `12`, cache read `0`, cache creation `0`, cost USD `0.000501`, context window `200000`, max output `32000`.
   - `claude-opus-4-7[1m]`: input `6`, output `6`, cache read `21351`, cache creation `11073`, cost USD `0.08006175`, context window `1000000`, max output `64000`.
   - `total_cost_usd`: `0.08056275`.

7. Derived calculations:
   - Top-level main-model processed-plus-output token count: `6 + 11073 + 21351 + 6 = 32436`.
   - Across `modelUsage`, raw processed-plus-output tokens: `453 + 32436 = 32889`.
   - Main model context-window percentage for this run, including cache buckets and output: `32436 / 1000000 * 100 = 3.2436%`.
   - Haiku side model context-window percentage: `453 / 200000 * 100 = 0.2265%`.
   - Main model output max percentage: `6 / 64000 * 100 = 0.009375%`.

8. Account/quota denominator check:
   - `claude auth status` reports login/auth/subscription metadata, including subscription type, but does not expose account usage used, account usage remaining, token quota, dollar quota, reset window, or percentage.
   - `claude --help` command list does not show a usage/quota/remaining command.

## Planned User-Facing Conclusion

Claude Code JSON gives us strong per-run accounting:

- per-run input tokens;
- cache creation tokens;
- cache read tokens;
- output tokens;
- per-model token buckets;
- estimated cost in USD;
- model names;
- context windows;
- max output tokens;
- latency/turn/session metadata.

So yes, if the desired percentage is "how much of the model context window did this answer use?", the JSON contains enough data to calculate it. For the probe, the main model used about `3.2436%` of its `1,000,000` token context window under a simple raw-token formula.

But if the desired percentage is "how much of my Claude subscription usage did this answer use, and how much subscription usage remains?", the JSON is still missing the denominator and remaining balance. It does not include a subscription quota, remaining usage, reset time, or account percent used. Tokens alone are not enough unless we know the plan's real quota formula and reset window.

The practical conclusion is:

- We can update the Claude review helper to save JSON and append per-run token/cost/context-window accounting.
- We cannot honestly append exact Claude account usage percentage remaining from the currently inspected Claude CLI JSON alone.
- If the user has an external known quota denominator, then the calculation is trivial: `answer_usage_percent = answer_tokens / quota_tokens * 100`, and `remaining_percent = (quota_tokens - cumulative_tokens) / quota_tokens * 100`.

## Scope Caveats

- I skipped a second `stream-json` probe because normal JSON already exposed the per-run usage fields; running a second generation would spend more Claude usage without a clear missing field.
- I will not expose the user's email or organization ID from `claude auth status` in the final answer.
- No repo code or process scripts were changed.

## Requested Review Scope

Review whether this conclusion accurately distinguishes per-run token/context accounting from account subscription quota accounting, and whether it avoids overclaiming what the JSON supports.
