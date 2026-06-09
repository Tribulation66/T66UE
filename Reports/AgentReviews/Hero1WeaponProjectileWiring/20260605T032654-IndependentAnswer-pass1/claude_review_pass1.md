Result: OK

## Independent Answer

This is an Operator-implementable, process-governed build that Codex can proceed on and decision-gate where a hard blocker exists. No user-only decision blocks starting — the user already approved the visual model and authorized "create and wire these projectiles," and the prompt explicitly authorizes the gameplay/data changes "when feasible."

Codex should proceed in this order and report evidence per step:

1. **Data (Weapons.csv, four Hero 1 AOE rows).** Update to match the approved model: projectile pattern/count to Single/1 (black), three-lobe/3 (red), five-lobe/5 (yellow), large-single/1 full-contact (white). Reconcile the damage multipliers to the approved *relative-to-black* model (red 1.20×, yellow 1.50×, white 2.00× of black baseline). Flag explicitly that live rows currently carry 1.20/1.35/1.55/1.80 multipliers and 120/180/260/360 damage, so the change is a balance-affecting reconciliation, and state the exact before/after values in the report.

2. **Silhouettes / VFX.** All four rows currently bind to the *same* `NS_Hero1AxeAOE_MeshSlash`. The approved model needs four visually distinct footprints (1 / 3 / 5 crescents and a 2× large crescent). Authoring distinct Niagara systems is Unreal-owned and likely cannot be done headless — this is the expected **decision-gate**. Per the process rule, any temporary non-Niagara procedural placeholder for the primary silhouette must be called out as a substitution and gated. The flat PNGs are mockups only (manifest confirms) and are not implementation.

3. **Impact model.** `CombatVFXImpactContextContract.md` already supports the needed policies: declare `OnePrimary` (black), a multi-point primary policy (red 3 / yellow 5), and a per-contact full-primary policy (white). Implement the 100% primary / 50% body split with idol triggering only on primary points, preserving `ParentSourceID` for any idol chain. This is runtime gameplay code → needs compile/build + staged standalone proof with the contract's log fields (impact points, parity counters, damage-by-source).

4. **Do not commit/push.** Report verification evidence for every changed data/code/asset.

## Evidence Checked

- Visual target folder + `visual_target_manifest.md`: four PNGs present; manifest self-declares "Visual target mockups only … do not implement Niagara assets, damage geometry, or impact-context rules," and notes the data/multiplier/impact-point work is a later step.
- `Weapons.csv` Hero 1 rows: black Single/1 mult 1.20 dmg 120; red TwinFan/2 mult 1.35 dmg 180; yellow Single/1 mult 1.55 dmg 260; white Single/1 mult 1.80 dmg 360 — confirms mismatch with approved model.
- `CombatVFXBindings.csv`: all four Hero 1 AOE rows bind to the identical `NS_Hero1AxeAOE_MeshSlash` + `Hero1AxeAOESlashMechanismPacket` — no per-rarity silhouette differentiation yet.
- `CombatVFXImpactContextContract.md`: confirms named per-attack publication policies (OnePrimary/PerTarget/…) and required proof fields exist to express the 1/3/5/full model and primary-vs-secondary phases.

## Questions Or Blockers

- None requiring the user. The Niagara silhouette authoring is the expected feasibility decision-gate for Codex to declare with an exact blocker, not a user decision.
- One item to surface clearly in the report (not block on): the multiplier-semantics reconciliation. The approved "X% of black" is being applied as relative scaling; if the user intended to preserve the existing absolute multiplier ladder, that would differ — state the chosen interpretation explicitly so the user can correct it.

## Caveats

- Treat the PNGs strictly as approved *visual intent*, not assets to import as the runtime slash.
- The primary visible slash must remain Niagara-owned VFX; any actor-arranged or procedural stand-in needs an explicit substitution call-out per process.
- Damage authority stays in gameplay/data, never the Niagara visual; video capture alone does not satisfy the impact-context gate — runtime logs/validator output are required.
- I did not open every referenced mechanism packet line-by-line; Codex should confirm the AOE packet's declared publication policy before extending it to multi-point.
