You are Claude acting as Operator for T66 under Codex approval.

Working task:
Capture a fresh Unreal-owned video of the Hero 1 Bounce projectile proof at the standard enemy-visible gameplay angle. The previous video used the review/top-down camera and Pablo could not see the enemies or projectile clearly. This run is capture-only.

Operator:
Claude, model claude-opus-4-8, FullOperator after Codex approval.

Validator:
Codex validates the produced artifacts and final user-facing report.

Scope:
- Use `C:\UE\T66` as repo root.
- Read the approved scope in `Reports/AgentReviews/Hero1BounceStandardAngleCapture/codex_operator_approval.md`.
- Capture mode: `hero1axebouncevfxbinding`.
- Use the standard gameplay-style angle. Do not pass `-UseReviewCamera`.
- Save the video under `C:\UE\T66\Saved\VideoCaptures\Hero1BounceStandardAngle_20260529\`.
- Produce an evidence bundle with retained frames/contact sheet/manifest/ffprobe output.
- After capture, inspect the evidence enough to confirm enemies are visible and the angle is not the prior top-down review camera.
- Write `Reports/AgentReviews/Hero1BounceStandardAngleCapture/claude_operator_completion.md` with:
  - exact capture command,
  - video path,
  - evidence paths,
  - whether enemies are visible,
  - whether `-UseReviewCamera` was omitted,
  - key log markers for Bounce sequencing and damage proof,
  - any caveats.

Required process:
- Follow `AGENTS.md`, `OPERATOR_VALIDATOR_PROTOCOL.md`, `Gameplay/GAMEPLAY_AGENTS.md`, `Reports/AGENTS.md`, and Combat VFX capture rules.
- Use Unreal-owned capture only.
- This is capture/proof work, not implementation. No source-code, content, CSV, DataTable, or asset edits are approved.
- Do not use Git mutating commands or broad Git/LFS scans.
- Do not delete prior evidence.
- Do not patch code if the capture is not readable; report the blocker and stop.

Recommended capture shape:
Use `Scripts\CaptureT66GameplayVideo.ps1` with `-CaptureMode hero1axebouncevfxbinding`, 1280x720, around 120 frames at 30 fps, retained frames, `-EvidenceBundle`, and `-EvidenceAutoSelectFrames`. Do not include `-UseReviewCamera`. If needed, use the script's default camera for this mode, which applies gameplay-style preview camera settings.

Stop condition:
Fresh MP4 and evidence bundle exist and the completion note gives Codex enough to validate that this is the standard angle with enemies visible.
