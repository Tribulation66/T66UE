You are Claude, the T66 Operator. Codex is Validator. Work in `C:\UE\T66`.

This is a narrowly scoped revision for the already-approved Hero 1 DOT weapon task.
Use the existing approval artifact:
`Reports/AgentReviews/Hero1DOTWeapon/codex_operator_approval.md`

Task contract:
Working task:
Operator: Claude.
Validator: Codex.
Scope: fix and prove the Hero 1 DOT placeholder so it has one moving hero-to-target projectile, one data-authoritative DOT payload, and three small target-following visual marker spheres located on the hit enemy for the DOT duration. Mini/minigame systems are out of scope.
Stop condition: the marker attachment/alignment defect is fixed, focused compile is attempted, DOT proof capture/log evidence is attempted, and `Reports/AgentReviews/Hero1DOTWeapon/claude_completion_packet.md` is updated.

Validator finding that must be addressed:
- `Source/T66/Gameplay/T66DotMarkerVFX.cpp` currently does:
  `SetActorLocation(FollowTarget->GetActorLocation());`
  then
  `AttachToActor(FollowTarget, FAttachmentTransformRules::KeepRelativeTransform);`
- This can preserve a world-sized relative offset after attachment instead of keeping the marker actor on the target. That directly violates the "three markers spawn on the enemy" requirement.

Required fix:
- Keep the DOT semantics exactly as approved: one DOT payload only, no multiplied marker damage lanes.
- Fix marker attachment so the marker actor stays on the target with a near-zero target-relative root offset after attachment.
- Add or improve lightweight proof logging around marker spawn/alignment if needed so Codex can validate target location, marker actor location, and/or target-relative offset from logs. The logging must stay DOT-proof-focused and not add noisy global logs.
- Do not alter final Niagara art or imagegen assets.
- Do not touch Mini/minigame systems.
- Do not revert unrelated user or prior-agent work.

Verification to attempt:
- Focused compile for `T66Editor Win64 Development`.
- Unreal-owned gameplay video capture using `Scripts/CaptureT66GameplayVideo.ps1 -CaptureMode hero1axedotvfxbinding`.
- If the video is still visually contaminated by the cream wall-occlusion rectangle or the markers are not readable, say that explicitly in the packet and provide the best available structural/log proof. Do not call weak visual evidence FULL.

Packet update requirements:
- Update `Reports/AgentReviews/Hero1DOTWeapon/claude_completion_packet.md`.
- First non-empty line must remain exactly:
  `Operator Packet: COMPLETE`
- Include changed files, exact commands, result markers, evidence paths, any remaining caveats, and Claude token availability.
- The packet must not claim full visual acceptance unless the capture clearly proves one visible moving projectile and three markers on the target after hit.
