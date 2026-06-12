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
- Packet path: C:\UE\T66\Reports\AgentReviews\CombatVFXWaterIdolImpactCaptureQuestion\answer_review_packet.md
- Output scope: review of the packet below only.

<review_packet>
# Water Idol Impact Capture Question - Answer Review Packet

## Working Goal

Determine whether the captured Water idol video actually used the Hero 1 AOE weapon path, explain why the AOE weapon projectile is not visible, and identify the correction needed so idol video proof shows a weapon projectile impact driving the idol overlay.

## User Concern

The user reviewed the delivered Water idol impact video and asked whether the AOE weapon was actually equipped. They expect idols to work only on top of weapons and to be triggered at the impact point of the weapon projectile. They do not see the AOE weapon projectile in the video.

## Applicable Instructions

- `AGENTS.md`: substantive answers require Claude review unless opted out.
- `Gameplay/GAMEPLAY_AGENTS.md`: combat work is owned by Gameplay/Combat and must follow combat docs.
- `Gameplay/Combat/VFX_PROCESS_INDEX.md`: combat VFX tasks route through the VFX process docs.
- `Gameplay/Combat/CombatVFXDefinitionOfDone.md`: gameplay capture must be Unreal-owned; selected frames/contact sheets do not by themselves prove visual fidelity.
- `Gameplay/Combat/CombatVFXIdolOverlayArchitecture.md`: weapon VFX owns the primary attack silhouette; idol overlay is additive and Water's first proof validates a fallback placeholder branch, not a real idol Niagara asset.
- `Gameplay/Combat/MASTER_COMBAT.md`: production-path automation proof must use real weapon selection, RunState inventory/item stats, combat fire, VFX binding lookup, and damage paths.

## Live Evidence Checked

Capture command in `Saved/Logs/T66.log` used:

- `-T66GameplayAutoCapture=hero1axeaoewateridolimpact`
- `-T66Hero1AxeAOEProofIdol=Idol_Water`
- `-T66Hero1AxeAOECenterPlayer`
- `-T66Hero1AxeAOEHitboxFireDelay=7.6`

Runtime log evidence:

- `[Hero1AxeAOEHitboxProof] EquippedAoeWeapon=Hero_1_black_aoe Success=1 HeroID=Hero_1`
- `[Hero1AxeAOEIdolImpactProof] EquippedProofIdol=Idol_Water`
- `[Hero1AxeAOEVFXBindingProof] ManualLabVFX=0 Reason=ProductionBindingDispatcherWillSpawnOnFire IdolImpactProof=1`
- `CombatImpactContext Phase=WeaponPrimary SourceType=WeaponBase SourceID=Hero_1_black_aoe ... AttackCategory=AOE ImpactPoint=V(X=360.00, Z=64.00) ... ImpactPointValid=1`
- `CombatVFXProductionSpawned Binding=Hero1Axe_AOE_Base SourceType=WeaponBase SourceID=Hero_1_black_aoe ... System=/Game/VFX/Hero1/Axe/AOE/NS_Hero1AxeAOE_MeshSlash.NS_Hero1AxeAOE_MeshSlash`
- `CombatImpactContext Phase=IdolPrimary SourceType=IdolModifier SourceID=Idol_Water ParentSourceID=Hero_1_black_aoe ... ImpactPoint=V(X=360.00, Z=64.00)`
- `CombatVFXIdolImpactPlaceholderSpawned SourceType=IdolModifier SourceID=Idol_Water ParentSourceID=Hero_1_black_aoe ... Placeholder=BlueSphere`
- `DamageBySource SourceID=Idol_Water TotalDamage=47 ProofIdol=Idol_Water`

Source evidence:

- `Scripts/CaptureT66GameplayVideo.ps1` lines 188-216 add hitbox timing, proof idol, debug view, and impact-source verbose logging for `hero1axeaoewateridolimpact`.
- `Source/T66/Gameplay/T66PlayerController_Overlays.cpp` lines 2904-2915 equip `Hero_1_black_aoe` in hitbox/idol proof modes.
- `Source/T66/Gameplay/T66PlayerController_Overlays.cpp` lines 3192-3243 skip manual lab VFX for Water idol proof and rely on the production binding dispatcher.
- `Source/T66/Gameplay/T66CombatComponent.cpp` lines 1712-1735 publish the weapon AOE impact context and spawn bound weapon base slash VFX.
- `Source/T66/Gameplay/T66CombatComponent.cpp` lines 2564-2640 create the Water idol impact context from the primary weapon impact point, then spawn bound idol VFX or fallback placeholder and apply idol-owned damage.
- `Content/Data/CombatVFXBindings.csv` line 2 binds `Hero1Axe_AOE_Base` to `Hero_1_black_aoe` / `AOE`, with `bSuppressTemporaryProjectile=True`.
- `Source/T66/Gameplay/T66CombatComponent.cpp` lines 1072-1086 suppress the old temporary weapon projectile when a binding says `bSuppressTemporaryProjectile`.

Visual evidence:

- Delivered contact sheet selected frames 50, 56, 64, and 68. Those frames mostly show the Water blue sphere placeholder and post-impact state.
- Direct inspection of `C:\UE\T66\Saved\VideoCaptures\Hero1AxeAOE_WaterIdolImpact_UserVideo_20260528\frames\frame_0047.png` shows the red/blue AOE slash arc briefly visible behind the red debug volume and Water blue sphere.
- Direct inspection of `frame_0046.png` and `frame_0048.png` shows the capture was visually cluttered by debug volume and placeholder timing.

## Codex Draft Answer

Yes, the video was captured with the Hero 1 black AOE weapon equipped and fired through real combat, not a standalone idol trigger. The logs prove `EquippedAoeWeapon=Hero_1_black_aoe Success=1`, `CombatImpactContext Phase=WeaponPrimary SourceID=Hero_1_black_aoe`, then `IdolPrimary SourceID=Idol_Water ParentSourceID=Hero_1_black_aoe` at the same impact point.

But the user's visual criticism is valid. The delivered video is not a good proof artifact for "idol on top of weapon" because the visible AOE weapon effect is a very brief red/blue Niagara slash, not the old temporary projectile, and the frame selection hid it under the Water placeholder/debug geometry. `Hero1Axe_AOE_Base` has `bSuppressTemporaryProjectile=True`, so the old blue AOE temporary projectile is intentionally absent once the production Niagara binding is active. If the user expects a distinct moving/impact weapon carrier in the proof, the capture needs to show the production slash clearly before/under the Water placeholder, or the capture harness needs a reviewed proof-only visibility mode.

Best correction: recapture, not reinterpret this video. Use the same real `hero1axeaoewateridolimpact` combat path, but adjust proof visibility: select frames around 45-48, reduce or delay the Water placeholder for proof, reduce debug clutter, and include a side-by-side base-only Hero 1 AOE capture plus base+Water idol capture. We should not re-enable the deprecated temporary projectile as the production answer because the binding intentionally suppresses it and the idol architecture says weapon VFX owns the primary silhouette.

## Reviewer Request

Please review the draft answer. Check for:

- incorrect claims about whether the weapon was equipped,
- incorrect distinction between production Niagara slash and temporary projectile,
- missing caveats about the video being a weak visual proof,
- unsafe recommendation to re-enable deprecated temporary projectile visuals,
- any contradiction with the Combat VFX/idol process docs.

Return first non-empty line exactly `Verdict: APPROVE`, `Verdict: REVISE`, `Verdict: NEEDS_HUMAN_DECISION`, or `Verdict: BLOCK`.

</review_packet>
