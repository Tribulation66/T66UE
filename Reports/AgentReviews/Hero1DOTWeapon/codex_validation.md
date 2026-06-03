Codex Validation: ACCEPTED WITH PLACEHOLDER-ART CAVEAT

Task: Hero 1 DOT weapon placeholder structure.
Operator: Claude.
Validator: Codex.

Validated changes:
- `PerformDOT` now spawns one visual-only hero-to-target shot, then applies one authoritative DOT payload on arrival.
- `AT66DotMarkerVFX` creates three visual-only marker spheres and snap-attaches the marker root to the hit target.
- The marker attachment defect found by Codex (`KeepRelativeTransform` after world placement) was fixed with `SnapToTargetNotIncludingScale`.
- DOT damage remains a single `HeroPrimaryDot` payload. The three marker spheres do not own or multiply damage.

Verification accepted:
- Focused compile: `Reports/AgentReviews/Hero1DOTWeapon/compile_output3.log` shows `Result: Succeeded`.
- Structural proof log: current `Saved/Logs/T66.log` shows one `T66DotShotSpawned`, one `T66DotApplicatorMarkersSpawned MarkerCount=3`, one `T66DotPayloadApplied Source=HeroPrimaryDot`, and `T66DotMarkerAlignment OffsetSize=0.000`.
- Corrected standard-angle capture: `Saved/VideoCaptures/hero1axedotvfxbinding_20260529_232228/hero1axedotvfxbinding.mp4`.
- Manual evidence strip: `Saved/VideoCaptures/hero1axedotvfxbinding_20260529_232228/evidence_manual/contact_sheet.png`.

Visual caveat:
- The DOT projectile and three spheres are intentional temporary placeholders, not final Niagara art.
- The proof capture stretches and enlarges the temporary visual carrier for readability. Runtime gameplay keeps the near-instant default unless `T66.DOT.ProofReadableTravelSeconds` is set for proof.

Rejected evidence:
- The earlier `Saved/VideoCaptures/hero1axedotvfxbinding_20260529_231442/` capture is not accepted as visual proof because it was filmed from behind a tower-wall occlusion block.

PPF CLOSE
Process used: Combat VFX placeholder structure using the repo gameplay capture process plus log-backed damage authority proof.
Matches declared process: YES for the temporary structure; final DOT Niagara art remains deferred by scope.
Evidence: compile log, corrected MP4, manual contact sheet, and DOT source/alignment log markers listed above.
