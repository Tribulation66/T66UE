# Physics Archive Index

This folder indexes historical physics/ragdoll evidence. It does not move or delete the original report artifacts.

Current implemented behavior is documented in:

```text
../CURRENT_STATE.md
../HISTORY.md
```

## Archive Policy

- Historical reports stay in their original `Reports/AgentReviews/...` folders unless a separate reference audit proves they are safe to move.
- Old approaches should be marked here and in `../HISTORY.md` before they are used as evidence.
- Do not delete old assets, reports, or runtime code without user-approved cleanup scope.
- Do not treat an archived design plan as implemented current behavior without checking live source.

## Historical Evidence

- `Reports/AgentReviews/FallGuysPhysicsArchitectureAssessment/`
  - Status: historical design assessment.
  - Use: explains why launch-only knockback was insufficient and why broad Physics ownership was created.

- `Reports/AgentReviews/FallGuysFullPhysicsRoadmapReview/`
  - Status: historical roadmap seed.
  - Use: records the proposed capsule-backed always-on active-ragdoll direction. This is not the current implementation.

- `Reports/AgentReviews/FriendSlopUnrealRagdollImport/`
  - Status: historical/prototype seed.
  - Use: contains FriendSlop PhysicsAsset and passive ragdoll import evidence.

- `Reports/AgentReviews/FriendSlopRagdollReassessment/`
  - Status: historical analysis.
  - Use: records failure modes around stretching and poor PhysicsAsset behavior.

- `Reports/AgentReviews/FriendSlopRagdollCameraFollow/`
  - Status: historical prototype fix.
  - Use: contains camera/follow lessons for hit-triggered ragdoll proof.

- `Reports/AgentReviews/FriendSlopRagdollFollowGroundGuard/`
  - Status: historical prototype fix.
  - Use: contains floor/follow guard lessons for ragdoll proof.

- `Reports/AgentReviews/HeroPACDisableDecision/`
  - Status: superseded policy decision.
  - Use: explains why PAC was disabled for one earlier pure-Chaos direction.

- `Reports/AgentReviews/ShortHeroRagdollIncap/`
  - Status: superseded tuning pass.
  - Use: short-incap values are not final feel settings.

- `Reports/AgentReviews/HeroActiveRagdollStage3Plan/`
  - Status: superseded implementation plan.
  - Use: documents the intended PAC/hip-anchor active-ragdoll approach.

- `Reports/AgentReviews/HeroActiveRagdollStage3Implementation/`
  - Status: superseded implementation attempt.
  - Use: historical context for the active-ragdoll state-machine direction.

- `Reports/AgentReviews/HeroRagdollSystemDiagnostic/`
  - Status: historical diagnostic.
  - Use: records why small setting tweaks were not enough.

- `Reports/AgentReviews/HeroRagdollStage3AuthorityRebuild/`
  - Status: superseded authority-model rebuild.
  - Use: historical context for the PAC/hip-anchor model. Live source must be checked before reusing this approach.

- `Reports/AgentReviews/PhysicsMasterAuditHandoff/`
  - Status: historical handoff.
  - Use: broad journey/context packet from before the current docs cleanup.
