Verdict: REVISE

## Blockers
- None hard. The plan is read-only on gameplay/content and only drives an existing capture script.

## Major Issues
- Flag names are paraphrased, not exact. `retained PNG frames`, `evidence bundle/contact sheet enabled`, and `Hero 1 AOE proof staging flags already used by the prior validated proof script` are descriptions, not parameters. Codex should resolve the exact `Scripts\CaptureT66GameplayVideo.ps1` switch names (e.g. `-KeepFrames`, `-EvidenceBundle`, `-ContactSheet`, plus the exact `-CVar`/`-ExecCmds` strings used by the prior Water idol proof run) before invocation, so the captured run is reproducible from the plan alone.
- The plan inherits "the prior validated proof script" by reference but does not pin which run that is. Without a concrete path or commit reference for the previously validated invocation, "staging flags already used" is not auditable. Codex should name the source command/log it is copying from.
- Capture mode existence is asserted but not verified. `hero1axeaoewateridolimpact` and `-Hero1AxeProofIdol Idol_Water` should be confirmed against the current script (the script is in the working tree's modified set per `git status`) before scheduling the run, in case parameter names drifted with the recent VFX pipeline codification.

## Minor Issues
- Output directory template `Hero1AxeAOE_WaterIdolImpact_UserVideo_<timestamp>` is fine but the script may have its own naming convention; if so, prefer the script's default rather than overriding, to keep evidence discoverable by existing tooling.
- No explicit capture duration or wait-for-impact bound. If the prior proof run had a known duration, state it so a hung run is detectable.
- No mention of whether a standalone build is current. The Out Of Scope clause defers a rebuild "unless the capture command itself reveals a blocking runtime issue", which is acceptable, but the plan should at least note the assumption that the last build artifact is fresh enough to reflect the Water idol impact structure under review.

## Clarifying Questions
- Should the delivery include the contact sheet image alongside the MP4, or only the MP4 path? The packet says "report the absolute MP4 path and evidence path" but the user only asked for "a video".
- Is there an expectation to also capture the Earth neutral control for comparison even though the user did not ask, or is single-clip Water-only correct? The plan chose Water-only — confirm this matches intent.

## Required Verification
- Capture command exits 0.
- MP4 exists, non-empty, and plays.
- Evidence bundle / contact sheet exists at the expected path.
- Frame inspection confirms blue Water placeholder sphere at slash impact point.
- Report absolute MP4 path (and evidence path) to the user.
- Add: log line showing `T66.Combat.ImpactSourceVerbose 1` took effect and the impact source was logged as the Water idol, to prove the structure (not just the visual placeholder) is firing.

## Rationale
The plan is scoped safely (no edits, existing script, clear out-of-scope), and the verification steps cover the user-visible deliverable. However, it leans on paraphrased flag names and an unnamed "prior validated proof script" rather than committing to exact parameters. Codex should tighten the invocation by reading `Scripts\CaptureT66GameplayVideo.ps1` and pinning the precise switches, cvars, and exec commands, then rerun review. Once the command is concrete and the capture mode is confirmed to still exist in the modified script, this is safe to execute.

