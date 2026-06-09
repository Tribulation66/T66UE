User request:

Ok what should be done next?

Task contract:

```text
Working task:
Operator: Codex
Validator: Claude
Scope: answer what should be done next after the FriendslopStyle process/tooling fix, without making implementation changes.
Stop condition: give a concrete next-step sequence that respects the new process and avoids repeating the prior false-positive fidelity pass.
```

Current state:

- The previous FriendslopStyle Main Menu implementation was judged not good enough.
- The pipeline was amended so structured verifier PASS is not enough.
- `VerifyUIFidelity.py` now supports containment checks and visual scorecard gates.
- The amended Main Menu checklist makes the prior capture fail (`PASS=96 FAIL=6 UNSURE=0`) instead of falsely passing.
- The user is asking for the next move, not asking for implementation yet.

Ask:

Give a concise repo-grounded recommendation for the next work step. Focus on practical sequencing and what should be proven before scaling to the full Main Menu again.
