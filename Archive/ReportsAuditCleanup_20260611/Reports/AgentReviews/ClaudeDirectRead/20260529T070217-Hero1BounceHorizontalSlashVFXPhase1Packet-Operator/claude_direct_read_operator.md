Phase 1 complete — doc-only, within approved scope. No code, CSV, asset, script, commandlet, capture, or Git operations were touched.

**What changed:** `Gameplay/Combat/Hero1AxeBounceMechanismPacket.md` rewritten from infrastructure scaffold into an active structural packet.

**Locked decisions (per approval):**
- Primary carrier: `ImpactAnchored` ArcSlash per chain link — small horizontal red/blue slash placed at each authoritative Bounce impact point.
- Context policy: `PerChainLink` (one official weapon impact context per chain link).
- Scaffold's prior `BeamHop` primary archetype demoted to optional, deferred connecting support, resolving the conflict with the user's slash wording.

**Sections included:** packet header/status with caveats, working goal, user constraints, process sources, carrier decision + rejections, PPF check, artifact parity gate, mechanism manifest, visual/damage alignment block, impact context contract block, anti-lookalike test, verification plan, mechanism close template, approval gate.

**Grounded against live code (read-only):**
- `PerformBounce` (T66CombatComponent.cpp:1928) currently builds **one aggregated** `BounceImpactContext` (`ChainIndex=0`, all handles in one context, published once) and spawns **no** bound production VFX. The packet records this as current state and flags that a later runtime phase must move it to `PerChainLink` — it does not edit runtime here.
- `TrySpawnBoundWeaponBaseSlashVFX` (line 1089) has AOE/Pierce dispatch but no Bounce branch yet.
- Legacy `SpawnBounceVFX` (lines 3101/3196/3239) named as the anti-lookalike path.
- No `Bounce` row in `CombatVFXBindings.csv` (confirmed).

**Rollback:** revert the single file; no runtime/asset side effects.

This is an Operator work artifact, not a greenlight — Codex should validate the packet for carrier/context/process completeness.

Codex Approval Required before any later phase (runtime spawn wiring, `PerChainLink` context change, Niagara/material/mesh assets, CSV/DataTable binding, commandlets, or capture).
