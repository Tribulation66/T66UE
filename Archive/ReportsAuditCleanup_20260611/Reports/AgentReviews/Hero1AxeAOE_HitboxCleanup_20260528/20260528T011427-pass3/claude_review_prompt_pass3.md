You are Claude reviewing a Codex implementation or answer plan for the T66 Unreal project.

Rules:
- Start your response immediately with the verdict line. Do not write any
  preface, summary, confirmation, Markdown rule, or other text before it.
- Do not edit files.
- Do not run commands.
- Do not implement the plan.
- Review only the packet below.
- Be strict about contradictions with repo instructions, missing verification, unsafe scope, and unclear goals.
- Treat Codex as the implementer and you as the reviewer.

The first non-empty line of your review must be exactly one of these three lines:
Verdict: APPROVE
Verdict: REVISE
Verdict: BLOCK

After that verdict line, return a concise Markdown review with exactly these headings:
Blockers
Major Issues
Minor Issues
Clarifying Questions
Required Verification
Rationale

Only use Verdict: APPROVE when the reviewed plan/output is safe for Codex to present as greenlit. If implementation still requires user go-ahead under AGENTS.md, APPROVE means safe to present at that go-ahead gate, not permission to skip the gate.

Review scope:
- Packet path: C:\UE\T66\Reports\AgentReviews\Hero1AxeAOE_HitboxCleanup_20260528\plan_packet_pass3.md
- Output scope: review of the packet below only.

<review_packet>
# Hero 1 Axe AOE Hitbox Alignment And VFX Cleanup Plan Packet - Pass 3

## Working Goal

Align the Hero 1 axe AOE VFX hitbox/debug outline with the visual, clean and organize the VFX process/assets/docs for future agents, and prepare a new-agent handoff for item-stat confirmation and future idol overlay VFX work.

## Review Context

Claude pass 1 and pass 2 both returned `Verdict: REVISE`:

- `Reports/AgentReviews/Hero1AxeAOE_HitboxCleanup_20260528/20260528T010311-pass1/claude_review_pass1.md`
- `Reports/AgentReviews/Hero1AxeAOE_HitboxCleanup_20260528/20260528T011000-pass2/claude_review_pass2.md`

This pass resolves the remaining review points:

- cites the existing primary-target exemption path,
- changes the PPF gate to acknowledge the user-go-ahead requirement instead of waiving it,
- tightens validator and `.uasset` gates,
- states the baseline radius source and defaults.

## User Constraints

- Current visual design is accepted enough for now; do not continue visual-design polish in this pass.
- Focus first on making the hitbox match the visual with the debug line around it.
- Then clean loose ends and organize the VFX tree/docs so future agents can follow the system.
- Prepare a new-agent prompt for later item/stat confirmation, scale confirmation, and idol overlay VFX design.
- Do not implement idol overlays or final item/stat proof in this pass.
- Default Mini/minigame scope remains excluded.

## Applicable Instructions Read

- `AGENTS.md`: active goal, live repo first, no process substitution, PPF/artifact/mechanism gates, Claude review gate, Unreal-owned capture, pending issues.
- `Gameplay/GAMEPLAY_AGENTS.md`: gameplay folder routing, combat VFX procedure, data-authored tuning, runtime verification expectations.
- `Gameplay/README.md`: combat docs are under `Gameplay/Combat`.
- `Gameplay/Combat/CombatVFXAuthoringProcedure.md`: combat VFX source/process authority, editor/gameplay evidence, hitbox proof separation.
- `Gameplay/Combat/CombatVFXInfrastructureInventory.md`: current VFX binding/capture/inventory state.
- `Gameplay/Combat/pending_issues_Combat.md`: existing known VFX proof/capture limitations.
- `Reports/AGENTS.md`: review/handoff reports go under `Reports/AgentReviews`, proof/handoff summaries under `Reports/Proof` or durable report paths.

## Current Live Findings

- `UT66CombatComponent::PerformSlash` computes `EffectiveSlashRadius` from combat/weapon/item scale and currently calls `BuildSlashTargets(..., InnerRadius=0.f)`.
- `BuildSlashTargets` already supports an `InnerRadius`, and applies it to both target filtering and `T66CombatDebugDraw::DrawDamageSector`.
- Existing primary-target exemption:
  - `Source/T66/Gameplay/T66CombatComponent.cpp:1402` adds `QueryPrimaryTarget` to ignored actors for the overlap query.
  - `Source/T66/Gameplay/T66CombatComponent.cpp:1422-1426` adds `QueryPrimaryTarget` to `OutTargets` before overlap filtering.
  - Result: the attack-selected primary target remains hit even when at the center of the crescent hollow; non-primary targets inside the hollow are filtered by `T66IsPointInsidePlanarSector`.
- `T66Hero1AxeAOEVFXCommandlet.cpp` builds the accepted crescent mesh from profile samples at lines 926-936 and renderer layer scales at lines 82-116.
- Recomputed from those exact source constants:
  - mesh min inner radius: `222.684 cm`,
  - mesh max outer radius after tangent offsets/ripples: `380.900 cm`,
  - largest slash layer renderer scale: `1.08`,
  - visible base outer carrier radius: `411.372 cm`,
  - chosen crescent inner ratio: `0.54`.
- `Content/Data/CombatVFXBindings.csv` currently has `BaseVisualRadius=435.0`. This will be changed to `411.4` so `EffectiveSlashRadius / BaseVisualRadius` maps the production Niagara carrier outer edge to the logical outer radius.
- Exact weapon row: `Content/Data/Weapons.csv` row/key `Hero_1_black_aoe`.
- `0.0` remains the schema sentinel for a filled-sector AOE. Nonzero values mean crescent-band AOE geometry.
- Baseline expected `EffectiveSlashRadius ~= 437.5 cm` comes from current runtime proof, not visual constants:
  - `Gameplay/Combat/Hero1AxeAOESlashMechanismPacket.md` records baseline proof log `EffectiveSlashRadius=437.52`.
  - Code formula: `BaseSlashRadius * ProjectileScaleMultiplier + WeaponAoeRadius` in `Source/T66/Gameplay/T66CombatComponent.cpp:1588-1590`.
  - Current data inputs for `Hero_1_black_aoe`: `SlashRadius=300.f` fallback, weapon `AttackScaleMultiplier=1.05`, weapon `BonusAoeRadius=120.0`, plus the run-state proof multiplier that produced the logged `437.52`.

## PPF Check

```text
PPF CHECK
Objective: Make the Hero 1 axe AOE logical damage hitbox/debug line match the accepted crescent slash visual, then clean docs/handoff for future VFX work.
Proven process: AGENTS.md Section 2 + Gameplay/Combat/CombatVFXAuthoringProcedure.md + Hero1AxeAOESlashMechanismPacket.md hitbox proof and production-binding close.
My planned implementation: Keep Niagara as presentation and combat as authority. Add combat-owned crescent inner-radius ratio metadata to weapon data for `Hero_1_black_aoe`, use it in BuildSlashTargets for the logical hitbox and debug outline, calibrate the production VFX BaseVisualRadius to the commandlet-measured carrier outer extent, rerun data import/validator/build, capture a hitbox proof, then update docs and handoff.
Same method class: YES
If NO, why:
User approval required before proceeding: YES. Claude approval only greenlights the plan; implementation still needs the active user-goal continuation/go-ahead gate.
Verification evidence: source diff, data table reload, production binding validator, focused T66Editor build, Unreal-owned hitbox/debug gameplay capture, proof log showing expected inside-band hits and outside/inner-hole misses, updated docs/handoff.
```

## Artifact Parity Gate

```text
ARTIFACT PARITY GATE
Reference artifact/category: Hero 1 AOE production Niagara crescent slash and combat DamageVolume debug sector.
Role: Primary
Required: YES
Planned artifact/path: /Game/VFX/Hero1/Axe/AOE/NS_Hero1AxeAOE_MeshSlash plus combat-owned crescent-band sector metadata in weapon data and debug draw from BuildSlashTargets.
Status: SAME
Evidence: production binding row, commandlet mesh profile, capture frame with debug outline around the visual, HP-delta proof log.
```

```text
ARTIFACT PARITY GATE
Reference artifact/category: Future-agent VFX process tree and handoff.
Role: Secondary
Required: YES
Planned artifact/path: Gameplay/Combat docs updates plus a durable handoff prompt under Reports.
Status: SAME
Evidence: edited docs/index pointers and saved handoff file.
```

## Mechanism Manifest

```text
MECHANISM MANIFEST
Reference/source: CombatVFXAuthoringProcedure.md, Hero1AxeAOESlashMechanismPacket.md, live T66CombatComponent.cpp, T66Hero1AxeAOEVFXCommandlet.cpp.
Required mechanisms:
  1. Mechanism: Shared visual/logical geometry contract
     Required: YES
     Planned implementation: Add `AoeInnerRadiusRatio` to `FWeaponData` with default `0.0`; set `Hero_1_black_aoe` to `0.54`; all other generated AOE rows explicitly `0.00`; derive hitbox inner radius from `EffectiveSlashRadius * AoeInnerRadiusRatio`; set `BaseVisualRadius=411.4`.
     Evidence needed: source/data diff, validator log, combat geometry log, and debug frame showing outer and inner sector lines around the crescent.
  2. Mechanism: Damage authority remains combat-owned
     Required: YES
     Planned implementation: Keep damage selection in `UT66CombatComponent`; do not use Niagara collision or VFX actor overlap.
     Evidence needed: HP-before/after proof log with expected hits/misses.
  3. Mechanism: Primary-target exemption remains intentional
     Required: YES
     Planned implementation: Preserve existing `QueryPrimaryTarget` pre-add path while non-primary overlap targets use the crescent-band filter.
     Evidence needed: proof rows where `Primary` at `(0,0)` hits and `InnerHollow` at `Forward * 120` misses.
  4. Mechanism: Item/stat scale still flows through `EffectiveSlashRadius`
     Required: YES
     Planned implementation: Keep existing `EffectiveSlashRadius` computation and derive inner radius from it, so later item scale changes scale both visual and hitbox proportionally.
     Evidence needed: code inspection now; next-agent prompt includes dedicated item-stat scale proof.
  5. Mechanism: Repeatable proof capture
     Required: YES
     Planned implementation: Update `hero1axeaoevfxbinding` proof target positions for crescent-band geometry, then capture through `Scripts/CaptureT66GameplayVideo.ps1` or `Scripts/RunHero1AxeAOEVFXBindingProof.ps1`.
     Evidence needed: MP4/evidence bundle/log excerpt.
  6. Mechanism: Clean future-agent process routing
     Required: YES
     Planned implementation: Update the infrastructure inventory and mechanism packet with the geometry contract, current tree, cleanup status, and next-agent prompt.
     Evidence needed: docs diff and report path.
```

## Anti-Lookalike

- Cheapest wrong result: leave the filled 180-degree sector hitbox and only change the visual scale until the outer line looks less wrong.
- Discriminator: proof must show both outer and inner debug boundaries for the crescent-band shape, plus at least one target inside the visible band hit and at least one non-primary target inside the old filled-sector hollow not hit.

## Planned Edit Scope

Code/data:

- `Source/T66/Data/T66DataTypes.h`: add `AoeInnerRadiusRatio = 0.f` to `FWeaponData`; document `0.0` as filled-sector sentinel.
- `Scripts/SetupWeaponsDataTable.py`: add `AoeInnerRadiusRatio` to generated CSV fields; set only row `Hero_1_black_aoe` to `0.54`, all other generated weapon rows including all other AOEs to `0.00`.
- `Content/Data/Weapons.csv`: generated source CSV will include `AoeInnerRadiusRatio`; exact changed nonzero row is `Hero_1_black_aoe`.
- `Content/Data/DT_Weapons.uasset`: expected generated DataTable touch after setup script reload.
- `Source/T66/Gameplay/T66CombatComponent.cpp`: compute `EffectiveSlashInnerRadius`, pass it to `BuildSlashTargets`, preserve primary-target exemption, and log `EffectiveSlashInnerRadius`/`AoeInnerRadiusRatio` with production VFX spawn or adjacent combat geometry proof logging.
- `Source/T66/Gameplay/T66PlayerController_Overlays.cpp`: automation-only proof target positions, no normal overlay runtime behavior change.
- `Content/Data/CombatVFXBindings.csv`: change `Hero1Axe_AOE_Base` `BaseVisualRadius` from `435.0` to `411.4`.
- `Content/Data/DT_CombatVFXBindings.uasset`: expected generated DataTable touch after setup script reload.
- `Scripts/ValidateCombatVFXProductionBindings.py`: add guards for field presence, `Hero_1_black_aoe == 0.54`, all other AOE rows explicitly `0.00`, `BaseVisualRadius=411.4`, source fragments, and proof log fragments when applicable.

Docs/reports:

- `Gameplay/Combat/Hero1AxeAOESlashMechanismPacket.md`: record crescent-band hitbox alignment close and proof paths.
- `Gameplay/Combat/CombatVFXInfrastructureInventory.md`: update infrastructure inventory and tree status.
- `Gameplay/Combat/MASTER_COMBAT.md`: update combat debug/hitbox contract for Hero 1 AOE crescent-band sector.
- `Gameplay/Combat/pending_issues_Combat.md`: update or leave existing manual-frame-window issue depending on new proof outcome; add any out-of-scope cleanup found.
- `Reports/Proof/CombatVFX/Hero1AxeAOE_HitboxCleanup_20260528/HANDOFF_NEXT_AGENT.md`: next-agent prompt.
- `Reports/Proof/CombatVFX/Hero1AxeAOE_HitboxCleanup_20260528/CLEANUP_STATUS.md`: current VFX tree and cleanup status.

Expected generated `.uasset` paths:

- `Content/Data/DT_Weapons.uasset`
- `Content/Data/DT_CombatVFXBindings.uasset`

No other `.uasset` changes are planned. The narrow status gate fails the run if any other `.uasset` appears changed, until inspected and explicitly accepted.

Out of scope:

- Idol overlay system implementation.
- Final visual polish of the accepted AOE slash.
- New VFX for DOT/Pierce/Bounce.
- Staged standalone packaging unless source/build verification shows it is required by the final scope.
- Mini/minigame systems.

## Proof Target Contract

The proof centers the attack on the primary target exactly as current combat does. Let `Forward` be from hero to primary target, and `Right` be its right vector. Proof offsets below are local to the primary target / slash center.

Expected baseline approximate values after this change:

- outer radius: about `EffectiveSlashRadius ~= 437.5 cm` in baseline proof, from current proof log `437.52`,
- inner radius: about `236.3 cm` from `0.54 * EffectiveSlashRadius`.

Target specs for `hero1axeaoevfxbinding`:

| Label | Local offset | Expected | Reason |
|---|---:|---|---|
| `Primary` | `(0, 0)` | Hit | Primary target remains hit by existing pre-add attack selection path. |
| `InsideBandForward` | `Forward * 320` | Hit | Inside visible crescent band. |
| `InsideBandSide` | `Forward * 270 + Right * 170` | Hit | Inside radius and within 180-degree arc. |
| `InnerHollow` | `Forward * 120` | Miss | Non-primary target inside old filled sector but inside new hollow. |
| `OutsideBehind` | `-Forward * 300` | Miss | Outside 180-degree frontal sector. |
| `OutsideRadius` | `Forward * 520` | Miss | Past outer radius. |

The proof succeeds only when expected-hit targets take damage, expected-miss targets do not, and the captured frame shows both outer and inner debug boundary lines around the slash.

## Git/LFS Check Procedure

Avoid broad Git/LFS scans. Use narrow checks as pass/fail gates:

- Before and after data reload, run `git status --short -- Content/Data/Weapons.csv Content/Data/DT_Weapons.uasset Content/Data/CombatVFXBindings.csv Content/Data/DT_CombatVFXBindings.uasset Source/T66/Data/T66DataTypes.h Source/T66/Gameplay/T66CombatComponent.cpp Source/T66/Gameplay/T66PlayerController_Overlays.cpp Scripts/SetupWeaponsDataTable.py Scripts/ValidateCombatVFXProductionBindings.py Gameplay/Combat Reports/Proof/CombatVFX/Hero1AxeAOE_HitboxCleanup_20260528`.
- After reload, run `git diff --numstat -- Content/Data/DT_Weapons.uasset Content/Data/DT_CombatVFXBindings.uasset`.
- Fail the run for cleanup if any unexpected `.uasset` appears outside the two expected DataTables, unless inspected and explicitly documented.

## Verification Plan

1. Reload `DT_Weapons` with `Scripts/SetupWeaponsDataTable.py`.
2. Reload `DT_CombatVFXBindings` with `Scripts/SetupCombatVFXBindingsDataTable.py`.
3. Run `Scripts/ValidateCombatVFXProductionBindings.py` through Unreal and confirm geometry-contract guards pass.
4. Build focused `T66Editor Win64 Development`.
5. Run an Unreal-owned `hero1axeaoevfxbinding` hitbox/debug proof capture with the proof target contract above.
6. Confirm proof log has `PASS` for `Primary`, `InsideBandForward`, `InsideBandSide`, `InnerHollow`, `OutsideBehind`, and `OutsideRadius`.
7. Confirm proof log includes `EffectiveSlashInnerRadius > 0` for `Hero_1_black_aoe`.
8. Inspect evidence contact sheet or selected frame for visible outer and inner debug lines around the current crescent visual.
9. Update docs/handoff with exact evidence paths and remaining next-agent tasks.

## Next-Agent Handoff Preview

The handoff will tell the next agent to:

- start from this hitbox cleanup close and current docs,
- confirm normal item/stat wiring rather than proof-only deterministic grants,
- verify AOE scale items increase both logical crescent hitbox and production Niagara visual size using the exact `EffectiveSlashRadius` and `AoeInnerRadiusRatio` contract,
- verify AOE attack-speed and damage item effects still show readable playback and correct damage numbers,
- design the idol overlay system as additive `IdolModifier` binding rows/effect packets layered over weapon-base VFX,
- avoid changing the temporary placeholder projectile system except where production bindings explicitly suppress it,
- use imagegen/mockup gates and source-method gates for new idol overlays,
- not implement new visual polish until the backend/stat/idol overlay seams are reviewed.

## Rollback

Rollback is data-first:

- restore `Hero_1_black_aoe` `AoeInnerRadiusRatio` to `0.00` for filled-sector gameplay,
- restore `Hero1Axe_AOE_Base` `BaseVisualRadius` to `435.0` only if a gameplay capture shows the accepted visual underfills the debug outer boundary after the `411.4` calibration,
- rerun the two DataTable setup scripts and validator.

## Codex Position Before Claude Review

Claude was correct that pass 2 needed to cite the primary-target exemption and restore the user-go-ahead requirement. The planned implementation now preserves the existing selected-primary behavior while making non-primary targets respect the crescent-band hollow. The proof contract explicitly distinguishes primary center hit from non-primary hollow miss. The validator and narrow `.uasset` gates are pass/fail requirements, not advisory notes.

</review_packet>
