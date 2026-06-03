Working task:
Operator: Claude.
Validator: Codex.
Scope: Packet-only revision for Hero 1 DOT weapon. Do not edit source, scripts, docs, data, or proof artifacts except `Reports/AgentReviews/Hero1DOTWeapon/claude_completion_packet.md`.
Stop condition: The completion packet passes the Packet Completeness Gate.

Issue:
The existing completion packet contains useful evidence, but it failed the requested packet shape because the first line is not exactly `Operator Packet: COMPLETE` or `Operator Packet: INCOMPLETE`.

Revise only:
- `Reports/AgentReviews/Hero1DOTWeapon/claude_completion_packet.md`

Required first line:
- Use `Operator Packet: COMPLETE` only if the packet truthfully covers implementation and attempted verification.
- Otherwise use `Operator Packet: INCOMPLETE`.

Required sections:
- Summary of changes.
- Files touched.
- PPF close.
- Mechanism close.
- Visual/damage alignment close.
- Impact context close.
- Exact verification commands and results.
- Video/log/evidence paths.
- Skipped verification and why, including visual readability limitations.
- Token usage if exposed.

Do not change implementation. This is packet revision only.
