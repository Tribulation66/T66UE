The recommendation is written to the plan file. Here's the concise version for Codex to validate.

## Recommendation: Combat VFX impact-context process gates

**Root gap:** None of the eight docs require a weapon to *publish* an impact context, an idol to *consume* one, a distinct *damage source* with `ParentSourceID`, or the structured diagnostic that proves it. Every existing gate is about visuals/assets/binding/alignment — so a future weapon or idol can ship without the Water behavior and still pass everything. The Water proof is a one-off, not a reusable gate.

**Smallest durable set** (mirrors the existing `CombatVFXVisualDamageAlignmentContract.md` precedent — one focused contract doc owns it, others add short reference rows):

1. **New owner doc `CombatImpactContextContract.md`** — defines publication (weapon), consumption (idol), damage-source identity, element-isolation/no-leakage, no-legacy-fallback, a **generalized element-parameterized diagnostic schema** (so future idols don't reinvent `Water*` field names), and the verbatim "video-is-not-proof" clause. Plus an `IMPACT CONTEXT CLOSE` template.
2. **DefinitionOfDone.md** — four new matrix rows (publication, consumption, damage-source identity, element isolation); extend `PARTIAL` to trip when the diagnostic or neutral-control run is missing.
3. **EffectPacketTemplate.md** — new "Impact Context Contract" section every packet fills (role, expected counts, SourceID/ParentSourceID, idol query radius, verbatim diagnostic, neutral control).
4. **AuthoringProcedure.md** — additive §3.5 gate + close-template line + one §14 pitfall (counters, not video, are authority).
5. **VFX_PROCESS_INDEX.md** — add to read order + standard flow + idol baseline row.
6. **IdolOverlayArchitecture.md** + **VisualDamageAlignmentContract.md** — cross-reference only (alignment impact point must equal the published/logged context impact point).

**Every weapon must prove:** publishes ≥1 context with SourceID/impact-point(=alignment point)/center/extents/category; `Eligible==expected`; `LegacyFallbacks=0`; diagnostic emitted.
**Every idol must prove:** `Expected==actual` consumption, skips=0, distinct SourceID + `ParentSourceID`, own damage source/radius, `ContextParity=PASS`, `LegacyFallbacks=0`, and a **neutral/other-element control run emitting zero element diagnostics**.
**Anti-video wording:** a placeholder sphere can spawn from a wrong/legacy path and still look right on camera — the structured `CombatIdolImpactDiagnostic` counters plus the neutral control are the authority, not the video.

**Deferred (tooling, not docs):** add a `pending_issues_Combat.md` entry to extend `ValidateCombatVFXProductionBindings.py` to assert the counter set and require the control artifact — same pattern as the already-noted alignment-validator gap.

**Two things Codex must verify before locking:** (a) the generalized schema field names match what the live runtime actually prints, and (b) the non-drift rule is respected (no per-effect numeric values copied into the generic contract).

Full detail is in the plan file. I held off on `ExitPlanMode` since this is a read-only operator artifact for Codex to integrate — let me know if you'd like me to adjust the recommendation or formally exit plan mode.

