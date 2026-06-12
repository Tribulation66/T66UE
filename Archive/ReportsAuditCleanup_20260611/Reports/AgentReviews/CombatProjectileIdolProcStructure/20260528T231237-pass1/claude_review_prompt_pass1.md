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

### End-of-Video Frame

Path: `Saved/VideoCaptures/Hero1AxeAOE_WaterIdolAreaAlignment_Final_20260528/frames/frame_0107.png`

The end frame shows trap/enemy combat clutter and `Trap Projectile Damage` labels, after the temporary Water proof sphere has expired. It does not, by itself, prove a second Hero 1 AOE weapon hit without a Water idol proc.

## Code Findings

### Finding 1: Current Water idol path is single-primary-context, not per weapon impact context

`Source/T66/Gameplay/T66CombatComponent.cpp`

- Lines 1541-1544 allocate `WeaponImpactContexts` plus a separate `PrimaryWeaponImpactContext`.
- Lines 1611-1616 append every weapon context but only preserve the first primary context.
- Lines 2537-2546 can fire an immediate Overclock second weapon attack, which can publish another weapon context.
- Lines 2586-2666 process Water idol impact presentation only once per idol slot, using `PrimaryWeaponImpactContext`.

Assessment: this is the structural mismatch with the user's concept. If one auto-attack can produce more than one weapon impact context, the idol layer should be driven by each eligible weapon impact context, not by the first saved primary context only.

### Finding 2: Visual projectile lanes are spawned before damage resolution

`Source/T66/Gameplay/T66CombatComponent.cpp`

- Lines 2451-2495 calculate and spawn weapon/idol visual projectile lanes before the weapon attack category path runs.
- Water suppresses its old temporary projectile lane at lines 2461-2470 and 2488-2492.
- The actual weapon damage and impact-context publication happen afterward, via `PerformPierce`, `PerformSlash`, `PerformBounce`, or `PerformDOT`.

Assessment: under normal locked-target rules this is usually fine because `TryFire` already requires a valid target and unblocked path at lines 2429-2446. But architecturally, visuals are not yet owned by a single "confirmed weapon impact" event. That makes it possible for future weapons/idols/capture modes to show presentation before the idol proc path has been resolved.

### Finding 3: Water has a context path, other idols still use legacy weapon-hit payload behavior

`Gameplay/Combat/MASTER_COMBAT.md` already states Water is the first impact-context consumer and other idols keep the legacy weapon-hit payload behavior.

Assessment: if the intended combat model is all-encompassing for every idol and every weapon, the current structure is only a first Water seam. The next structural fix should centralize idol proc dispatch from weapon impact contexts.

### Finding 4: There is an old fallback that can apply idol damage even when no explicit weapon hit actor was recorded

`Source/T66/Gameplay/T66CombatComponent.cpp`

- Lines 2573-2575 add `PrimaryTarget` to `WeaponHitActors` when `WeaponHitActors` is empty.

Assessment: this matches "locked attacks should hit" for current gameplay, but it is not a clean event contract. A better system should derive idol procs from confirmed `FT66CombatImpactContext` records, and use an explicit fallback reason only for categories that intentionally have no context yet.

## Proposed Next Fix Direction

1. Treat `WeaponImpactContexts` as the authoritative list of weapon impacts for this attack tick.
2. Add a small internal helper, conceptually `ProcessIdolImpactFromWeaponContext(const FT66CombatImpactContext& WeaponContext, const FCachedIdolSlot& IdolSlot)`.
3. For Water AOE, build one `IdolImpactContext` per eligible `WeaponContext`, preserving:
   - `ParentSourceID = WeaponContext.SourceID`
   - `ImpactPoint = WeaponContext.ImpactPoint`
   - `DamageCenter = ImpactPoint` for the current Water AOE proof
   - `PrimaryTargetHandle = WeaponContext.PrimaryTargetHandle`
   - hit target handles from the Water radius query
4. Iterate `WeaponImpactContexts` for every impact-presentation idol instead of checking only `PrimaryWeaponImpactContext`.
5. Keep the legacy `WeaponHitActors` path only for idols not yet migrated to impact contexts.
6. Add logs for skipped impact-context idol procs, such as no valid impact point, no equipped idol, or no valid hit targets, so future videos cannot be ambiguous.
7. Add or update the proof harness to intentionally exercise a multi-impact case, such as Overclock double fire, and prove that two weapon contexts create two Water idol contexts.

## Recommendation

The user is right at the architecture level: a fired locked-on weapon attack should produce a confirmed weapon impact, and an equipped idol should proc from that impact. The first Water proof hit did proc correctly, but the implementation is not yet the all-encompassing event pipeline. It still has a single-primary-context Water path, pre-resolution visual spawning, and legacy per-hit idol handling.

The next implementation pass should convert Water's proc from `PrimaryWeaponImpactContext` to the full `WeaponImpactContexts` list and make that helper the reusable pattern for future idols/weapons.

## Questions For Validator

1. Is the diagnosis above supported by the cited live code/logs?
2. Is there any code path in this inspected area that already guarantees one idol proc per weapon impact context?
3. Should the next fix be implemented as a per-context helper over `WeaponImpactContexts`, or is there a smaller safer seam?
4. Are there any caveats the user should hear before implementation?

</review_packet>
