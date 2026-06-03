Codex Approval: APPROVE

Task: Hero 1 DOT marker cadence fix.

Approved Operator: Claude.
Validator: Codex.

Approved scope:
- Change the temporary Hero 1 DOT placeholder marker actor so the three target-attached DOT marker spheres reveal sequentially after impact, with a 0.5 second cadence between markers.
- Preserve the existing one moving DOT shot, one target, one authoritative DOT payload, and one `HeroPrimaryDot` damage source.
- Preserve marker-only visual authority: the DOT marker spheres must keep no collision, no damage ownership, and no independent gameplay authority.
- Add or update proof logs that make marker reveal order and delay inspectable.
- Update the DOT mechanism packet/docs so future DOT placeholder/final-art work knows that multi-dot applicator markers must reveal over time instead of appearing simultaneously.
- Attempt focused compile and Unreal-owned DOT proof capture through the existing capture process.

Explicit exclusions:
- Do not implement final DOT Niagara art.
- Do not change DOT damage tuning, target acquisition, idol overlay behavior, or Mini/minigame systems.
- Do not use destructive git operations or broad asset scans.

Required evidence:
- Compile attempt result.
- Unreal-owned DOT capture path, or clear reason capture could not run.
- Log evidence showing marker reveal index/order/timing and a single `HeroPrimaryDot` DOT payload.
- Operator packet with first non-empty line `Operator Packet: COMPLETE`.
