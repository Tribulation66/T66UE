Verdict: APPROVE

Blockers
- None.

Major Issues
- None. Scope is read-only capture, command is copied verbatim from a previously validated wrapper, and parameters match documented script support.

Minor Issues
- The plan reconstructs the command inline instead of invoking the existing wrapper `Scripts\RunHero1AxeAOEWaterIdolImpactProof.ps1`. That's defensible because the user-facing run needs different output paths/labels, but it means parameter drift from the wrapper won't be caught automatically. Consider noting any deltas vs. the wrapper for future-proofing.
- "Inspect the proof log" steps don't name the absolute log path that the script emits for this run. The evidence bundle should contain it, but stating the expected log path (or how to locate it under `EvidenceRoot`) tightens verification.
- `-EvidenceSelectedFrames` indices (start=50, mid=56, impact=64, dissipate=68) are inside `-FrameCount 72` but leave the pre-impact buildup unsampled (frames 0–49). If the user wants the swing telegraph visible in the contact sheet, add an earlier frame; if only the impact moment matters, current selection is fine.
- Output directory `Hero1AxeAOE_WaterIdolImpact_UserVideo_20260528` is not date-time stamped, so a re-run on the same day would collide. Low risk since this is a one-shot delivery, but worth noting.

Clarifying Questions
- None required for Codex to proceed; the user request ("capture and send a video of the Water idol impact proof in action") is unambiguous and the plan matches it.

Required Verification
- Capture script exits 0.
- MP4 exists at the declared `-Output` path and is non-empty.
- Evidence bundle exists under `-EvidenceRoot`, including the contact sheet.
- Contact sheet / selected frames show the blue Water placeholder sphere at the slash impact point.
- Proof log contains `CombatImpactContext Phase=IdolPrimary SourceType=IdolModifier SourceID=Idol_Water`.
- Proof log contains `CombatVFXIdolImpactBindingLookup ... SourceID=Idol_Water ... Result=None` (confirms current no-binding fallback path).
- Codex reports absolute MP4 path and evidence path back to the user.

Rationale
- Task is a pure capture using the repo's accepted Unreal-owned video capture path (`Scripts\CaptureT66GameplayVideo.ps1`), with no gameplay/content/code/data edits and no rebuild — consistent with combat VFX video-proof process and AGENTS scope.
- Parameters, capture mode (`hero1axeaoewateridolimpact`), and proof markers are copied from the most recently validated proof run, which substantially reduces execution risk.
- Verification covers process exit, artifact existence, visual evidence, and log markers — adequate for a single-shot user delivery.
- The few weaknesses (wrapper reuse, log path naming, frame selection coverage, directory collision risk) are quality-of-life polish, not safety issues, so the work can proceed and the notes can be folded in opportunistically.

