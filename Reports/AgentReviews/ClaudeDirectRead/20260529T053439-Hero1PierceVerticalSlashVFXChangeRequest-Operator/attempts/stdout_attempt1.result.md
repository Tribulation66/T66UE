# Operator Change Request / Operator Packet — Hero 1 Pierce Vertical Slash VFX

**Operator:** Claude · **Validator:** Codex · **Run type:** Planning/proposal only (no edits, no commands, no asset writes, no captures performed this run)

---

## 1. Working Task And Validation Depth

**Task:** Build the Chad 1 / Hero 1 Pierce weapon projectile VFX as a **forward vertical slash** that travels along the pierce lane, reusing the red/blue/white visual language and the reusable color/material/texture vocabulary from the existing Hero 1 AOE attack where appropriate.

**Validation depth:** Combat VFX production binding depth — proves binding structure, required assets, source guards, declared data contracts, and PathAnchored visual/damage alignment. **Does not** prove final visual fidelity or Pablo visual approval (that is a separate gate; see §6 imagegen decision and §10).

**First production target:** `Hero_1_black_pierce` (Weapons.csv: DamageMultiplier 1.20, AttackSpeed 1.03, Scale 1.05, Range 1.03, BonusHitDamage 3, BonusPierceCount 2, AoeInnerRadiusRatio 0.00, FalloffPerHitMultiplier 0.85). No live evidence indicates this target is unsafe.

---

## 2. Roles And Tool Profile

- **Operator (Claude):** authors packet now; on Codex approval, performs scoped edits, lab asset authoring, dispatcher wiring, binding-row addition, editor-isolation + gameplay capture, validator runs.
- **Validator (Codex):** runs Packet Completeness Gate; emits `Codex Approval: APPROVE` to `Reports/AgentReviews/Hero1AxePierceVerticalSlash/codex_operator_approval.md` before any full Operator run; later validates real output under `OPERATOR_VALIDATOR_PROTOCOL.md`.
- **Tool profile this run:** read-only. No mutating commands, no Unreal/commandlet, no imagegen, no capture.

---

## 3. User Constraints And Out Of Scope

**Constraints (verbatim from prompt):**
- "Do not edit files, run mutating commands, invoke Unreal/editor commandlets, generate assets, or capture video in this run. This run is for an Operator Change Request / Operator Packet only."
- "Do not ask multiple reworded questions. If one user-only decision blocks implementation, emit a decision gate and stop."

**Pablo's visual direction (verbatim):**
- "Pierce weapon projectile"
- "it should indeed be a slash but a vertical slash forward"
- "We can use the red and blue and white, same colors and materials/textures as the aoe attack."

**Out of scope:** Mini/minigame VFX; Hero 1 DOT/Bounce; idol overlays; repo-wide generated-asset policy; damage-number/balance changes; any non-Pierce binding.

---

## 4. Applicable Instructions Read

- `AGENTS.md` (root process router) · `Reports/AGENTS.md` (artifact routing) · `OPERATOR_VALIDATOR_PROTOCOL.md`
- `Gameplay/Combat/VFX_PROCESS_INDEX.md` · `CombatVFXAuthoringProcedure.md` · `CombatVFXDefinitionOfDone.md`
- `CombatVFXVisualDamageAlignmentContract.md` · `CombatVFXImpactContextContract.md`
- `Hero1AxePierceMechanismPacket.md` (existing infra scaffold) · `Hero1AxeAOESlashMechanismPacket.md` (reference precedent)
- `MASTER_COMBAT.md` · `CombatVFXInfrastructureInventory.md` · `CombatVFXGeneratedAssetPolicy.md`

---

## 5. Evidence And Live Findings

**Existing Pierce packet conflict (must reconcile):** `Hero1AxePierceMechanismPacket.md` declares the intended read as a *"straight horizontal axe-force slash or fissure that travels along a line"* with carrier *"ArcSlash, RibbonTrail, or BeamHop pending source evidence,"* binding deferred, no active production row. **Pablo's new direction ("vertical slash forward") overrides the "horizontal" framing.** The new packet records this as an approved direction change.

**Code seam (T66CombatComponent.cpp):**
- `TrySpawnBoundWeaponBaseSlashVFX(...)` (lines 1089–1219) is generic and AttackCategory-driven: resolves binding via GameInstance, computes `VisualScale = (EffectiveDamageRadius/BaseVisualRadius)*VisualScaleMultiplier`, clamps playback (`MinReadableSlashPlaybackSeconds = 0.20f`), spawns Niagara at `VisualPivot + Z70`, logs `CombatVFXProductionSpawned`. **Currently maps Radius/InnerRadius (AOE/BandAnchored/CenterAnchored) — no LineLength/TubeRadius (PathAnchored) path.**
- `PerformPierce` (lines 1677–1734): builds capsule line query (`PierceRadius = 80 * ProjectileScaleMultiplier`, `LineLength = AttackRange`), publishes `PierceImpactContext` with `ImpactPoint`, `Forward=Dir`, `LineLength`, `TubeRadius`, `bImpactPointValid=true`. **Does NOT call `TrySpawnBoundWeaponBaseSlashVFX` — this is the gap.**
- `PerformSlash` (lines 1737–1834) calls `TrySpawnBoundWeaponBaseSlashVFX(SlashImpactContext, EffectiveDamagePerShot, CurrentHeroID, AttackCategory)` at **line 1798** — the pattern to mirror.
- Dispatch switch (2528–2546) already routes `ET66AttackCategory::Pierce → PerformPierce`. Suppression check at line 2452 via `ShouldSuppressWeaponBaseProjectileVisual`.

**Data seam:** `CombatVFXBindings.csv` contains only `Hero1Axe_AOE_Base` (binds `Hero_1_black_aoe` AOE → `/Game/VFX/Hero1/Axe/AOE/NS_Hero1AxeAOE_MeshSlash`, BaseVisualRadius 411.4, BasePlaybackSeconds 0.46, VisualScaleMultiplier 1.0, bSuppressTemporaryProjectile True). A Pierce row must be added.

**Scripts are AOE-hardcoded:** `SetupCombatVFXBindingsDataTable.py` hardcodes the AOE required row (BaseVisualRadius 411.4); `ValidateCombatVFXProductionBindings.py` enforces `Hero1Axe_AOE_Base` + `BaseVisualRadius==411.4` and an all-AOE REQUIRED_ASSETS list. Both need Pierce-aware extension.

**Alignment contract:** Pierce is a forward line/lane → **PathAnchored** (anchor model: travels along Forward from official impact origin to LineLength; tolerance default path-endpoint within ~50 units of authoritative capsule query end). Damage authority remains the capsule line query in `PerformPierce`; VFX is presentation only.

---

## 6. PPF And Process Gates

- **PPF Check:** Source = Pablo's written direction (three quotes in §3) + AOE precedent reuse. No Pablo transcript beyond the quotes; the carrier *shape* (vertical forward slash) is new and not yet visually approved → see imagegen decision below.
- **Artifact Parity Gate:** Reuse AOE material/color/texture vocabulary (red/blue/white). New Pierce assets must reuse AOE master materials/parameter language; only mesh footprint + orientation (vertical plane, forward travel) and timing differ.
- **Mechanism Manifest:** Carrier = **PathAnchored ArcSlash oriented as a vertical blade-plane that sweeps/translates forward along the pierce lane** (a RibbonTrail variant is the fallback if the vertical plane reads poorly in motion). Mechanism = forward translation along `Forward` over `LineLength`, scaled by `ProjectileScaleMultiplier`.
- **Anti-Lookalike Discriminator:** see §11.
- **Mechanism Close:** deferred to full Operator run (requires editor-isolation + gameplay capture evidence).

**Imagegen-first scope decision (explicit):** The AOE precedent required an approved **same-view visual-target mockup** before Niagara acceptance. The Pierce carrier shape (vertical forward slash) **differs materially** from the AOE crescent even though material language is reused. **Operator recommendation: imagegen-first IS required** before *visual acceptance* — produce a same-view Pierce mockup for Pablo approval. **However, the structural/code/binding work (carrier wiring, CSV row, dispatcher path-anchored support, script extensions, lab scaffold) does NOT depend on the mockup** and can proceed on Codex approval. The mockup gates only the final visual-fidelity/Pablo-approval close, not the binding-structure proof.

---

## 7. Proposed Patch Approach (full Operator run, pending approval)

Per path: **path · planned change · why · blast radius · rollback.**

1. **`Gameplay/Combat/Hero1AxePierceMechanismPacket.md`**
   - Change: rewrite from infra scaffold to active packet — record vertical-forward-slash direction (override "horizontal"), set carrier = PathAnchored ArcSlash (RibbonTrail fallback), declare lab/production paths, parity reuse, alignment contract, binding row name, mechanism manifest, anti-lookalike, checkpoint/close structure mirroring the AOE packet.
   - Why: durable process doc; required by VFX flow before promotion.
   - Blast radius: docs only.
   - Rollback: git revert file.

2. **`Content/Data/CombatVFXBindings.csv`**
   - Change: add row `Hero1Axe_Pierce_Base` binding `Hero_1_black_pierce` / Pierce → `/Game/VFX/Hero1/Axe/Pierce/NS_Hero1AxePierce_MeshSlash`, with BaseVisualLength (path length baseline ≈ pierce AttackRange-derived), BaseTubeRadius (≈ 80 baseline), BasePlaybackSeconds, VisualScaleMultiplier=1.0, bSuppressTemporaryProjectile=True.
   - Why: data-driven production binding the resolver reads.
   - Blast radius: one new row; AOE row untouched; no row → no Pierce VFX (safe no-op).
   - Rollback: delete row, regen DT.

3. **`Content/Data/DT_CombatVFXBindings.uasset`** (regenerated)
   - Change: regenerate from CSV via setup script.
   - Why: runtime reads the DataTable, not the CSV.
   - Blast radius: binding table; AOE row preserved.
   - Rollback: regen from prior CSV.

4. **New lab assets `/Game/VFXLab/Hero1Axe/Pierce/`** (cook-excluded)
   - Change: author NS_/materials/mesh for vertical forward slash, reusing AOE master materials/params.
   - Why: lab-first authoring per process before promotion.
   - Blast radius: lab namespace only, not cooked.
   - Rollback: delete lab folder.

5. **New production assets `/Game/VFX/Hero1/Axe/Pierce/`** (NS_Hero1AxePierce_MeshSlash + materials + mesh)
   - Change: promote approved lab assets.
   - Why: binding row target.
   - Blast radius: new production folder; AOE assets untouched.
   - Rollback: delete folder + revert CSV row.

6. **`Source/T66/Gameplay/T66CombatComponent.cpp` — `PerformPierce`**
   - Change: after building `PierceImpactContext`, add a `TrySpawnBoundWeaponBaseSlashVFX(PierceImpactContext, EffectiveDamage, CurrentHeroID, AttackCategory)` call mirroring line 1798; gate behind existing suppression check.
   - Why: closes the spawn gap; Pierce currently spawns no bound VFX.
   - Blast radius: Pierce path only; if no binding row, resolver returns no system → existing no-op (safe).
   - Rollback: remove the call.

7. **`Source/T66/Gameplay/T66CombatComponent.cpp` — `TrySpawnBoundWeaponBaseSlashVFX` PathAnchored support**
   - Change: add a PathAnchored branch that scales by LineLength/TubeRadius (from impact context) instead of Radius/InnerRadius, and orients the carrier along `Forward`. Keep AOE branch unchanged.
   - Why: dispatcher currently only maps AOE Radius/InnerRadius; Pierce needs path/tube scaling + forward orientation.
   - Blast radius: adds a branch keyed on anchor type; AOE behavior unchanged.
   - Rollback: remove branch.

8. **`Scripts/SetupCombatVFXBindingsDataTable.py`**
   - Change: add Pierce required-row definition alongside AOE.
   - Why: setup must produce the Pierce row deterministically.
   - Blast radius: script + regenerated DT.
   - Rollback: revert script.

9. **`Scripts/ValidateCombatVFXProductionBindings.py`**
   - Change: make Pierce-aware — add Pierce REQUIRED_ASSETS, accept `Hero1Axe_Pierce_Base`, validate Pierce baselines without breaking AOE 411.4 enforcement.
   - Why: validator must pass with the new row and guard the new assets.
   - Blast radius: validation logic; AOE checks preserved.
   - Rollback: revert script.

---

## 8. Verification Plan (full Operator run)

1. `ValidateCombatVFXProductionBindings.py` passes with both AOE and Pierce rows.
2. Setup script regenerates DT; diff shows only the added Pierce row.
3. Editor-isolation capture (MRQ) of the Pierce carrier in lab staging — confirms vertical-plane forward-travel read.
4. Gameplay capture via `Scripts/CaptureT66GameplayVideo.ps1` with a Pierce preview staging — confirms in-combat readability and `CombatVFXProductionSpawned` log on Pierce.
5. PathAnchored alignment proof: VFX path endpoint within tolerance (~50 units) of the authoritative capsule line-query end; damage still authored solely by the capsule query (VFX presentation-only).
6. Anti-lookalike control (§11): Pierce visually distinct from AOE crescent in same-view capture.
7. Evidence bundle + logs to `Reports/Proof/CombatVFX/Hero1AxePierceVerticalSlash/`; decision block to `Reports/AgentReviews/Hero1AxePierceVerticalSlash/decision_block.md`.

---

## 9. Token Routing

- **Heavy mechanical/repetitive work → Codex** (manual-paste handoff): CSV row authoring, setup/validator script extensions, DT regen command sequence, capture command scaffolding, log-grep verification passes.
- **Operator-judgment work → Claude:** packet authoring, dispatcher PathAnchored branch design, alignment-contract reconciliation, carrier-archetype decision, anti-lookalike framing.
- **User-only:** imagegen visual-target approval (Pablo).

---

## 10. Operator Position And Open Decisions

**Operator position:** Pablo's direction is explicit enough to proceed with this packet and (on Codex approval) the full structural/code/binding build. **No hard NEEDS_HUMAN_DECISION blocker** prevents the structural work.

**Primary open decision (user-only, non-blocking for structure):** Approve an **imagegen same-view Pierce visual-target mockup** before final visual acceptance. Operator recommends imagegen-first for the *visual close* only. The carrier-shape choice (vertical blade-plane ArcSlash vs. RibbonTrail fallback) will be locked against that approved mockup.

**Recorded direction change:** existing Pierce packet "horizontal" → Pablo's "vertical slash forward" (override applied in the rewritten packet).

---

## 11. Anti-Lookalike Discriminator

The Pierce carrier must NOT read as the AOE attack despite shared material/color language. Discriminators:
- **Geometry/anchor:** AOE = CenterAnchored/BandAnchored crescent footprint around the impact; Pierce = **PathAnchored vertical blade-plane** that **translates forward** along the lane (`Forward` × LineLength). Lane motion is the primary tell.
- **Orientation:** AOE sweeps horizontally around a point; Pierce is a vertical plane oriented across the lane, advancing forward — no radial sweep.
- **Timing/footprint:** AOE scales by Radius/InnerRadius; Pierce scales by LineLength/TubeRadius. A static crescent ≠ a forward-traveling vertical slash.
- **Control test:** same-view capture of AOE and Pierce side-by-side must be unambiguously distinguishable in a single frame mid-motion.

---

**Stop condition met:** Complete Operator Packet produced for Codex approve/reject. No file writes, commands, captures, or asset generation performed. No NEEDS_HUMAN_DECISION blocker; one non-blocking user-only decision flagged (imagegen visual-target approval).

---

*Codex Token Spent: 0 (no Codex invocation this run)*
*Claude Tokens Spent: this planning run only — reads + packet authoring; no mutating operations*
