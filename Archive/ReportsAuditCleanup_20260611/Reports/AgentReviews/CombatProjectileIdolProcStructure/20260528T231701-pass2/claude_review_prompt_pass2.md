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

The first non-empty line of your review must be exactly one of these four lines:
Verdict: APPROVE
Verdict: REVISE
Verdict: NEEDS_HUMAN_DECISION
Verdict: BLOCK

After that verdict line, return a concise Markdown review with exactly these headings:
Blockers
Major Issues
Minor Issues
Clarifying Questions
Required Verification
Rationale

Verdict meanings:
- APPROVE: the reviewed plan/output is safe for Codex to proceed to implementation under the reviewed scope. Codex should not ask for redundant manual user approval after APPROVE unless the user explicitly marked the work planning-only, asked Codex to stop before implementation, the packet has an unresolved user-only decision, or AGENTS/PPF requires explicit approval for a method substitution.
- REVISE: Codex can resolve the issue by improving the plan/output, inspecting more repo state, tightening verification, changing implementation approach, or otherwise doing more Codex-owned work. Codex should revise and rerun review.
- NEEDS_HUMAN_DECISION: the plan/output depends on product direction, vision, risk acceptance, scope choice, or another decision only the user can make. Codex should save a decision block, ask once, and stop until the user answers.
- BLOCK: the plan/output cannot safely proceed because of a hard blocker, missing prerequisite, external-state issue, unavailable credential/context, or contradiction that is not solved by normal Codex revision.

Review scope:
- Packet path: C:\UE\T66\Reports\AgentReviews\CombatProjectileIdolProcStructure\structure_review_packet.md
- Output scope: review of the packet below only.

<review_packet>
# Combat Projectile Idol Proc Structure Review Packet

## Working Goal

Inspect the combat projectile-to-impact-to-idol-proc structure and determine why a weapon projectile can appear to trigger without the Water idol triggering when attacks should be locked-on hits.

## Scope

- Read-only analysis only; no code edits in this pass.
- User concern: in the final proof video, a weapon projectile/effect appeared near the end without the Water idol placeholder.
- Desired combat concept: auto attacks are locked to an enemy, enemies cannot dodge, every fired projectile should hit, and every weapon hit should trigger an idol proc.
- Mini/minigame scope excluded.

## Instructions Loaded

- `AGENTS.md`
- `Gameplay/GAMEPLAY_AGENTS.md`
- `Gameplay/Combat/VFX_PROCESS_INDEX.md`
- `Gameplay/Combat/CombatVFXAuthoringProcedure.md`
- `Gameplay/Combat/CombatVFXVisualDamageAlignmentContract.md`
- `Gameplay/Combat/CombatVFXIdolOverlayArchitecture.md`
- `Gameplay/Combat/pending_issues_Combat.md`
- `Reports/AGENTS.md`

## Live Evidence

### Final Proof Capture Log

Path: `Saved/VideoCaptures/Hero1AxeAOE_WaterIdolAreaAlignment_Final_20260528/T66.log`

The main scripted proof fire has both the weapon context and Water idol context:

- Line 975: `CombatImpactContext Phase=WeaponPrimary SourceID=Hero_1_black_aoe ... ImpactPoint=V(X=696.89, Z=64.00) ... HitTargets=3`
- Line 976: `CombatVFXProductionSpawned Binding=Hero1Axe_AOE_Base SourceType=WeaponBase SourceID=Hero_1_black_aoe ...`
- Line 977: `CombatImpactContext Phase=IdolPrimary SourceID=Idol_Water ParentSourceID=Hero_1_black_aoe ... HitTargets=5`
- Line 978: `CombatIdolWaterImpactResolved SourceID=Idol_Water ParentSourceID=Hero_1_black_aoe ... Targets=5`
- Line 980: `CombatVFXIdolImpactPlaceholderSpawned SourceID=Idol_Water ... VisualLocation=V(X=696.89, Z=64.00) Radius=300.00`
- Lines 995-996: damage log contains both `AutoAttack` and `Idol_Water`.

This means the first scripted proof hit did not miss the Water idol proc.

### End-of-Video Frame / Reported Symptom Check

Path: `Saved/VideoCaptures/Hero1AxeAOE_WaterIdolAreaAlignment_Final_20260528/frames/frame_0107.png`

The end frame shows trap/enemy combat clutter and visible `Trap Projectile Damage` labels, after the temporary Water proof sphere has expired. The late log window after the scripted Hero 1 proof contains no additional:

- `CombatImpactContext Phase=WeaponPrimary`
- `CombatVFXProductionSpawned Binding=Hero1Axe_AOE_Base`
- `CombatIdolWaterImpactResolved`
- `CombatVFXIdolImpactPlaceholderSpawned`

The only Hero 1 weapon impact context in the full capture log is the scripted proof fire at lines 975-976, and it is immediately followed by the Water idol context and placeholder at lines 977-980. Therefore the end-of-video visual does not currently prove a missed Water proc; it is more consistent with trap/enemy combat clutter after the proof sphere expired.

## Code Findings

### Finding 1: Observed capture did not miss the Water idol proc

The actual scripted Hero 1 AOE weapon hit in this capture generated both the weapon and Water idol contexts in the same frame range. User suspicion is reasonable from the video, but the log evidence does not support a real missed-proc event in that capture.

### Finding 2: Current Water idol path is single-primary-context, not per weapon impact context

`Source/T66/Gameplay/T66CombatComponent.cpp`

- Lines 1541-1544 allocate `WeaponImpactContexts` plus a separate `PrimaryWeaponImpactContext`.
- Lines 1611-1616 append every weapon context but only preserve the first primary context.
- Lines 2537-2546 can fire an immediate Overclock second weapon attack, which can publish another weapon context.
- Lines 2586-2666 process Water idol impact presentation only once per idol slot, using `PrimaryWeaponImpactContext`.

Assessment: this is a real architectural limitation, but it is not proven to be the cause of the observed end-of-video artifact. If one auto-attack can produce more than one weapon impact context, the idol layer should eventually be driven by each eligible weapon impact context, not by the first saved primary context only.

### Finding 3: Visual projectile lanes are spawned before damage resolution

`Source/T66/Gameplay/T66CombatComponent.cpp`

- Lines 2451-2495 calculate and spawn weapon/idol visual projectile lanes before the weapon attack category path runs.
- Water suppresses its old temporary projectile lane at lines 2461-2470 and 2488-2492.
- The actual weapon damage and impact-context publication happen afterward, via `PerformPierce`, `PerformSlash`, `PerformBounce`, or `PerformDOT`.

Assessment: under normal locked-target rules this is acceptable because `TryFire` already requires a valid target and unblocked path at lines 2429-2446. This is not evidence of a missed proc in the capture. It is a design consideration for a future fully centralized impact event pipeline.

### Finding 4: Water has a context path, other idols still use legacy weapon-hit payload behavior

`Gameplay/Combat/MASTER_COMBAT.md` already states Water is the first impact-context consumer and other idols keep the legacy weapon-hit payload behavior.

Assessment: if the intended combat model is all-encompassing for every idol and every weapon, the current structure is only a first Water seam. Future structure work should centralize idol proc dispatch from weapon impact contexts, but this is broader than explaining the current video.

### Finding 5: There is an old fallback that can apply idol damage even when no explicit weapon hit actor was recorded

`Source/T66/Gameplay/T66CombatComponent.cpp`

- Lines 2573-2575 add `PrimaryTarget` to `WeaponHitActors` when `WeaponHitActors` is empty.

Assessment: this matches "locked attacks should hit" for current gameplay, but it is not a clean event contract. A better system should derive idol procs from confirmed `FT66CombatImpactContext` records, and use an explicit fallback reason only for categories that intentionally have no context yet.

## Proposed Next Fix Direction

No immediate bug fix is justified solely by the final frame of the previous capture. The correct near-term action is diagnostic:

1. Recapture with end-of-video clutter reduced or disabled, or crop/shorten the capture after the proof window.
2. Add explicit log/proof counters for `WeaponImpactContextCount`, `WaterIdolImpactContextCount`, and skipped idol-impact reasons.
3. If the user wants the broader architecture tightened now, then implement the reusable per-context dispatch:
   - treat `WeaponImpactContexts` as the authoritative list of weapon impacts for this attack tick,
   - add a small internal helper, conceptually `ProcessIdolImpactFromWeaponContext(const FT66CombatImpactContext& WeaponContext, const FCachedIdolSlot& IdolSlot)`,
   - for Water AOE, build one `IdolImpactContext` per eligible `WeaponContext`, preserving:
   - `ParentSourceID = WeaponContext.SourceID`
   - `ImpactPoint = WeaponContext.ImpactPoint`
   - `DamageCenter = ImpactPoint` for the current Water AOE proof
   - `PrimaryTargetHandle = WeaponContext.PrimaryTargetHandle`
   - hit target handles from the Water radius query
   - iterate `WeaponImpactContexts` for every impact-presentation idol instead of checking only `PrimaryWeaponImpactContext`,
   - keep the legacy `WeaponHitActors` path only for idols not yet migrated to impact contexts,
   - prove a multi-impact case, such as Overclock double fire, creates matching Water idol contexts.

## Recommendation

The user is right at the combat-concept level: a fired locked-on weapon attack should produce a confirmed weapon impact, and an equipped idol should proc from that impact.

For the specific video, the evidence does not show a Hero 1 weapon hit without a Water proc. The only logged Hero 1 weapon hit did produce Water. The end-of-video visual appears to be trap/enemy combat clutter after the temporary Water sphere expired.

For the structure, the current implementation is only partial architecture: Water is driven from `PrimaryWeaponImpactContext`, not the full `WeaponImpactContexts` list, and other idols still use the legacy weapon-hit payload path. That is the next real pipeline improvement if the user wants the all-encompassing weapon/idol event contract now.

## Questions For Validator

1. Is the revised diagnosis supported by the cited live code/logs?
2. Does the evidence now adequately distinguish the observed end-of-video frame from a real Hero 1 missed Water proc?
3. Is it correct to report "no observed missed proc in that capture, but a real future architecture gap remains"?
4. If the user asks for implementation next, is the per-context helper over `WeaponImpactContexts` the right first structural pass, or should diagnostics/logging come first?

</review_packet>
