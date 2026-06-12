Implement Phase 1 from:

`Reports/AgentReviews/Hero1DOTProductionBinding/codex_operator_approval_phase1.md`

User request:
"Ok can we have the DOT weapon production row be active like the others? There is no reason it should not be."

Use FullOperator within the approved scope only.

Critical guard:
Do not satisfy this by adding a fake CSV-only row. The row may be active only if a real DOT weapon Niagara carrier exists under `Content/VFX/Hero1/Axe/DOT/` and `PerformDOT` resolves/uses that carrier for the single moving DOT shot. Runtime movement may transport the carrier; the DOT shot silhouette must be authored by the Niagara/material/renderer asset, not by actor-side temporary mesh geometry.

Preserve DOT behavior:
- initial contact damage stays synchronous;
- weapon impact context publication stays synchronous;
- exactly one `ApplyDOT(... HeroPrimaryDot)` payload remains;
- marker reveal cadence and 0.3s visible pulse stay unchanged;
- no idol production rows;
- no Mini/minigame scope.

Produce:
`Reports/AgentReviews/Hero1DOTProductionBinding/operator_packet_phase1.md`

First non-empty line:
`Operator Packet: COMPLETE`
