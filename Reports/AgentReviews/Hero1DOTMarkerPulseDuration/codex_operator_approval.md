Codex Approval: APPROVE

## Approved Task

Change the Hero 1 DOT placeholder markers so each temporary marker pulse disappears after a 0.3 second visible lifetime. The existing 0.5 second reveal cadence stays in place, so only one placeholder dot should be visible at a time in the current three-marker DOT proof.

## Approved Scope

- Update `Source/T66/Gameplay/T66DotMarkerVFX.h/.cpp` to add a per-marker visible duration of 0.3 seconds for these placeholder markers.
- Keep the current reveal order: marker 0 immediately, marker 1 at +0.5s, marker 2 at +1.0s.
- Hide each marker after its 0.3 second visible duration, with safe timer callbacks if the actor is destroyed first.
- Add stable proof logging for marker hide/disappear events, including marker index, marker count, planned reveal delay, and visible duration.
- Update `Gameplay/Combat/Hero1AxeDOTMechanismPacket.md` to document that this placeholder DOT marker pass uses 0.3 second pulses and does not require all future DOT visuals to use that exact duration unless their packet opts in.
- Preserve one moving DOT shot, one target, one authoritative `HeroPrimaryDot` DOT payload, and visual-only marker authority.

## Approved Tool Surface

Claude FullOperator through `Scripts\Invoke-ClaudeDirectRead.ps1` with this approval artifact. Focused source/docs edits, focused compile, and Unreal-owned DOT proof capture are approved.

## Required Process Rules

- Follow `AGENTS.md`, `OPERATOR_VALIDATOR_PROTOCOL.md`, `Gameplay/GAMEPLAY_AGENTS.md`, `Gameplay/Combat/CombatVFXAuthoringProcedure.md`, and `Reports/AGENTS.md`.
- Do not use native goal tools.
- Use Unreal-owned capture proof, not desktop screenshots.
- Keep Mini/minigame systems out of scope.
- Operator packet first non-empty line must be exactly `Operator Packet: COMPLETE`.

## Explicitly Excluded Actions

- Do not implement final DOT Niagara art.
- Do not change DOT damage tuning, target acquisition, idol behavior, or source identity.
- Do not change Bounce/Pierce/AOE weapon behavior.
- Do not run destructive git operations or broad Unreal asset/LFS scans.

## Verification Required After Operator Run

- Focused compile attempt for `T66Editor Win64 Development`.
- Unreal-owned DOT proof capture attempt with the standard `hero1axedotvfxbinding` command.
- Log evidence showing:
  - one `T66DotShotSpawned`,
  - one `T66DotApplicatorMarkersSpawned MarkerCount=3`,
  - three marker reveal lines at planned delays `0.00`, `0.50`, `1.00`,
  - three marker hide/disappear lines with visible duration `0.30`,
  - one `T66DotPayloadApplied ... Source=HeroPrimaryDot`,
  - marker alignment remains near zero offset.

## Approval Rationale

This is a bounded correction to the temporary DOT marker presentation. It preserves the current DOT structure and damage authority while fixing the visual pulse lifetime the user identified.
