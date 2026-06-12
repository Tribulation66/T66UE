Codex Approval: APPROVE

## Approved Task

Phase 2: capture the Hero 1 DOT weapon production-binding runtime proof video and evidence bundle.

## Approved Scope

Approved in scope:

- Run the existing Unreal-owned video capture route:
  `Scripts/CaptureT66GameplayVideo.ps1 -CaptureMode hero1axedotvfxbinding -UseHero1AxePreviewStaging -NoHero1AxeTargets -FrameCount 120 -EvidenceBundle -EvidenceAutoSelectFrames`
- Preserve the generated `Saved/VideoCaptures/<run>/` output folder.
- Inspect the produced MP4/evidence bundle/logs for:
  - moving single DOT shot;
  - bound carrier path `/Game/VFX/Hero1/Axe/DOT/NS_Hero1AxeDOT_MeshSlash`;
  - three staggered marker reveals at planned delays 0.00 / 0.50 / 1.00s;
  - marker hides with `VisibleDurationSeconds=0.30`;
  - one `T66DotPayloadApplied ... Source=HeroPrimaryDot (single payload)`;
  - usable camera angle without camera wall-occlusion proof contamination.
- Write `Reports/AgentReviews/Hero1DOTRuntimeCapture/operator_packet_phase2.md`.

Explicitly out of scope:

- Code/content/data/docs edits, except for report artifacts under `Reports/AgentReviews/Hero1DOTRuntimeCapture/`.
- Any gameplay behavior change.
- Any idol production row or Mini/minigame work.
- Git operations.
- Desktop screenshots as proof.
- Replacing the existing capture process with ad hoc recording.

## Approved Tool Surface

Claude FullOperator through `Scripts\Invoke-ClaudeDirectRead.ps1` with `-ToolProfile FullOperator`, `-PermissionMode bypassPermissions`, `-NoSessionPersistence`, and this approval artifact.

## Required Process Rules

- Follow `AGENTS.md`, `OPERATOR_VALIDATOR_PROTOCOL.md`, `Reports/AGENTS.md`, `Gameplay/GAMEPLAY_AGENTS.md`, `Gameplay/Combat/VFX_PROCESS_INDEX.md`, `Gameplay/Combat/CombatVFXAuthoringProcedure.md`, `Gameplay/Combat/CombatVFXDefinitionOfDone.md`, and `Gameplay/Combat/Hero1AxeDOTMechanismPacket.md`.
- Use Unreal-owned capture only.
- Current capture evidence is required; prior validation cannot satisfy this request.
- Do not treat the capture as final visual-polish approval; this is runtime structure proof.

## Verification Required After Operator Run

- MP4 path exists and is playable/non-empty.
- Evidence bundle exists with `ffprobe.json`, contact sheet, selected frames, manifest, and visibility checklist when produced by the script.
- Log contains DOT carrier, marker reveal/hide, and single payload proof.
- Operator packet first non-empty line must be `Operator Packet: COMPLETE`.

## Approval Rationale

The user explicitly asked for the video. This is proof-bearing Combat VFX work, so it routes to Claude FullOperator for the capture phase, with Codex validating the produced evidence before user-facing delivery.
