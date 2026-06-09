User request:

"Ok then do what needs to be done until its time for me to go in and tell you how to tune the feel"

Working task:
Operator: Codex
Validator: Claude
Scope: make the FriendSlop Hero 1 TestRoom active-ragdoll path stable enough for design feel tuning; stop before subjective bounce/feel tuning that needs the user's eye.
Stop condition: PAC-enabled TestRoom capture completes without hang/crash, logs PhysicalAnimation=1, shows impact/ragdoll/recovery, focused build passes, staged standalone is refreshed if playable content changes, and remaining subjective tuning knobs are clearly identified.

Relevant repo rules:
- Do not use native goal tools.
- Codex is Operator and Claude is Validator per .t66/operator-state.json.
- Follow OPERATOR_VALIDATOR_PROTOCOL.md.
- Use Unreal-owned capture proof, not desktop screenshots.
- Refresh staged standalone if playable content changes.
- Keep scope isolated to TestRoom/FriendSlop PAC stabilization unless a broader change is required for proof.
- Current known issue: Source/T66/Gameplay/pending_issues_Gameplay.md says passive ragdoll works, but PAC-enabled capture logged PhysicalAnimation=1 then wrote only 3 frames before timeout.
