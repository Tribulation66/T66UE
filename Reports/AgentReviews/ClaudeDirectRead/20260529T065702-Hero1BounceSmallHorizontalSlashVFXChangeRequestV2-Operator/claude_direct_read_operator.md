I have enough to produce the Operator Change Request. Key live findings are confirmed: no Bounce binding row, `PerformBounce` never spawns bound VFX, the spawn helper has no Bounce anchor branch, and a legacy pixel-chain `SpawnBounceVFX` is the lookalike to avoid.

---

# Operator Change Request — Hero 1 / Chad 1 Bounce VFX

## 1. Task Contract & Validation Depth
```
Working task: Implement Hero 1 Bounce production VFX as a small horizontal red/blue slash that
              strikes the primary locked enemy and chains to a second enemy, reusing the accepted
              AOE/Pierce slash material family.
Operator:    Claude (claude-opus-4-8), read-only this turn; FullOperator for approved mutating phases.
Validator:   Codex (per .t66/operator-state.json:1-7).
Scope:       Bounce only for Hero_1_black_bounce. Reuse AOE/Pierce material+mesh method class.
Stop:        Per-phase completion packets; final = bound Bounce slash spawns on primary + chain link,
             compile + datatable + validator + Unreal-owned capture, Pablo visual sign-off deferred.
```
- **Depth: FULL.** Process-governed Niagara combat VFX (AGENTS.md §4 "Niagara combat VFX") + runtime code change + new assets + binding data + capture proof. Broad → phase-bounded (OVP §"Phase-Bounded").
- Mini/minigame: out of scope. No idol/DOT/Pierce/AOE behavior changes.

## 2. Live Findings (anchors)
- **No Bounce binding row.** `Content/Data/CombatVFXBindings.csv:2-3` holds only `Hero1Axe_AOE_Base` and `Hero1Axe_Pierce_Base`. Weapon row exists: `Content/Data/Weapons.csv:3` (`Hero_1_black_bounce`, Branch=Bounce, BonusBounceCount=2, no AOE radius).
- **PerformBounce never spawns bound VFX.** `T66CombatComponent.cpp:1928-2005` applies damage, builds a local `ChainPositions` (1947-1949,1977) and publishes one impact context (2001), but — unlike Pierce (1821) and AOE (1889) — **never calls `TrySpawnBoundWeaponBaseSlashVFX`**. The local `ChainPositions` is vestigial in this lambda (unused).
- **Spawn helper has no Bounce branch.** `T66CombatComponent.cpp:1089-1234`: anchor models are CenterAnchored / BandAnchored (1127-1131,1149) and PathAnchored=Pierce only (1132-1133,1152-1171). It spawns **one** system at one location — no per-link/chain handling.
- **Bounce impact context = OnePrimary, no per-link points.** `T66CombatComponent.h:207-230`: context has single `ImpactPoint`, `ChainIndex`, and `HitTargetHandles[]`, but **no chain/link position array**. PerformBounce sets `ChainIndex=0` (1955) and adds all hits to `HitTargetHandles` (1965,1985).
- **Lookalike already in tree:** legacy `SpawnBounceVFX` segment/pixel chain (`T66CombatVFX.cpp:1078`, `TrySpawnHeroBounceVariantPixels:779-826`) is invoked by the temporary-projectile path (`T66CombatComponent.cpp:3085-3101`), not by PerformBounce. This is the cheap wrong result; the bound carrier must not be this.
- **Scaffold conflict (decision-relevant):** `Hero1AxeBounceMechanismPacket.md:13-16` currently declares primary carrier **`BeamHop`** ("spirit/aura axe silhouettes or hop trails"). The user's new direction ("small horizontal slash that hits… and chains") is a **primary-carrier change** → AGENTS.md §2 / AuthoringProcedure §4 require a written reason + re-review (the packet activation in Phase 1 IS that record).
- **Proven precedent:** Pierce was activated today the same way (`Hero1AxePierceMechanismPacket.md:1-4`, commandlet `T66Hero1AxePierceVFXCommandlet.h`, binding `CombatVFXBindings.csv:3`). Reuse this exact method class.

## 3. Phase Plan (Codex approves one phase at a time)
1. **Design/activate packet** (doc only) — resolve carrier decision; fill PPF/parity/mechanism/alignment/impact-context/anti-lookalike. *(first mutating phase, §5)*
2. **Runtime wiring** — add a Bounce ImpactAnchored spawn path; call it from `PerformBounce` per hit link; compile.
3. **Asset/commandlet** — `T66Hero1AxeBounceVFXCommandlet` builds horizontal-slash mesh + Niagara reusing AOE/Pierce slash materials; add binding CSV row; datatable refresh; validator.
4. **Capture proof** — `Scripts/CaptureT66GameplayVideo.ps1` gameplay video: slash on primary + chain to 2nd, multi-frame.
5. **Completion/validation** — integrate, final verification, Codex completion packet. Pablo visual sign-off deferred.

## 4. PPF / Artifact / Mechanism Summary (for the Bounce packet)
- **Proven process:** accepted Hero 1 AOE/Pierce Niagara+material+mesh pipeline (commandlet authors mesh+Niagara, promote, `SetupCombatVFXBindingsDataTable.py`, `ValidateCombatVFXProductionBindings.py`), PathAnchored/ImpactAnchored alignment + impact-context contracts. **Same method class: YES** for structural pass.
- **Primary carrier (recommended): `ArcSlash` per chain link, ImpactAnchored** — a small horizontal slash spawned at each struck enemy. Secondary (deferred): `RibbonTrail`/`BeamHop` hop-line between links, `SupportImpact`. This honors the user's literal "slash that hits each enemy" while reusing the AOE crescent/Pierce blade method class. *(Supersedes scaffold BeamHop; recorded in packet.)*
- **Artifact parity (Primary, required):** horizontal-slash mesh (`/Game/VFX/Hero1/Axe/Bounce/SM_…`), Bounce Niagara (`/Game/VFX/Hero1/Axe/Bounce/NS_Hero1AxeBounce_MeshSlash`), reused AOE slash materials, `Hero1Axe_Bounce_Base` binding row, runtime per-link spawn. Silhouette must live in Niagara/material/mesh — **not** the legacy `SpawnBounceVFX` pixel chain or actor geometry.
- **Mechanisms (required):** per-link impact placement (slash at each hit target); horizontal slash silhouette distinct from AOE crescent & Pierce vertical blade; AOE red/blue material reuse; visual/damage alignment (slash at authoritative bounce hit point); chain ordering (primary → 2nd). Material reveal/erosion = deferred polish. Temporal proof needs multi-frame capture (one still cannot prove the chain).

## 5. Proposed First Mutating Phase (for Codex approval)
**Phase 1 — Activate `Hero1AxeBounceMechanismPacket.md` (doc only, low risk).**
- **File:** `Gameplay/Combat/Hero1AxeBounceMechanismPacket.md` (full rewrite from scaffold → active packet, mirroring `Hero1AxePierceMechanismPacket.md` structure).
- **Edits:** Status→active; Working Goal (horizontal slash + chain); User Constraints; Process Sources; **Carrier Decision** (ImpactAnchored ArcSlash per link; written reason superseding BeamHop; rejected lookalikes incl. legacy `SpawnBounceVFX`); PPF check; Artifact Parity gate; Mechanism Manifest; Visual/Damage Alignment block (anchor=ImpactAnchored, per-link impact points, damage stays authoritative in PerformBounce); Impact Context block (publication policy decision: keep OnePrimary vs. move to PerChainLink — see §10); Anti-Lookalike; Verification commands; Approval gate (final visual deferred).
- **Also create:** `Reports/AgentReviews/Hero1BounceHorizontalSlashVFX/codex_operator_approval.md` (first line `Codex Approval: APPROVE`) before any FullOperator phase.
- **Commands:** none (doc edit). **Verification:** Codex reads packet for completeness vs. Pierce packet parity + contract coverage.
- **Exclusions:** no source/asset/CSV/datatable edits in Phase 1; no compile; no capture; no Mini.
- **Rollback:** revert the single markdown file (git restore of that path); no build/asset impact.

## 6. Imagegen / Visual Approval
**Not required before structural implementation.** Bounce reuses the already-approved AOE/Pierce red/blue slash material family (Pierce precedent deferred imagegen: `Hero1AxePierceMechanismPacket.md:63`). Final `FULL` visual-fidelity acceptance still requires Unreal-owned capture + Pablo sign-off. If Pablo wants a same-view target first, add the AuthoringProcedure §3.2 mockup gate before Phase 3.

## 7. Anti-Lookalike Discriminator
- **Cheap wrong result:** the legacy `SpawnBounceVFX` pixel/segment line between enemies (`T66CombatVFX.cpp:1078,779-826`), a recolored AOE crescent, or a static slash with no per-link placement.
- **Discriminator:** a Niagara/material/mesh **horizontal** slash carrier instantiated **at each struck enemy in chain order** (primary, then 2nd), visually distinct from the AOE radial crescent and the Pierce vertical blade; damage proven separately through `PerformBounce` combat queries (`T66CombatComponent.cpp:1958-1998`), not Niagara. One still cannot pass — capture must show the chain across frames.

## 8. Token Routing
```
TOKEN ROUTING
OperatorModel: claude-opus-4-8
OperatorTokensSpent: Unavailable (read-only operator turn; no helper manifest emitted)
OperatorRunDir: n/a
OperatorManifest: n/a
CodexApprovalPath: Reports/AgentReviews/Hero1BounceHorizontalSlashVFX/codex_operator_approval.md (to be created Phase 1)
ExpectedValidatorDepth: deepened (runtime code + new Unreal assets + binding data + capture; OVP escalation triggers)
ValidatorBudgetHint: verify PerformBounce lacks a bound-VFX spawn (1928-2005 vs Pierce 1821/AOE 1889);
  confirm spawn helper anchor branches (1089-1171); confirm no Bounce row (CombatVFXBindings.csv:2-3);
  confirm context has no per-link array (T66CombatComponent.h:207-230); confirm scaffold carrier=BeamHop.
```

**Open decisions for Codex/user:** (a) **carrier archetype** — confirm ImpactAnchored ArcSlash per link (recommended) vs. scaffold BeamHop; (b) **impact-context policy** — keep `OnePrimary` and drive per-link spawns from `HitTargetHandles[]`/`ChainPositions` (smaller change, no struct edit) vs. switch to `PerChainLink` with a context per link (cleaner contract, larger change + struct field). My recommendation: ImpactAnchored ArcSlash + keep OnePrimary, spawning per `HitTargetHandle` inside the existing PerformBounce loop.

**Read-only note:** this profile cannot compile, run, capture, or drive the editor; Phases 2-4 require a Codex-approved FullOperator run. No current-verification claim is made here.

**Codex Token Spent:** Unavailable

**Claude Tokens Spent:** Unavailable
