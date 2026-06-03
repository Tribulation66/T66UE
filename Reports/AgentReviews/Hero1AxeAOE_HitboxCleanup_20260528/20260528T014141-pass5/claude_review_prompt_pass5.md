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
- Packet path: C:\UE\T66\Reports\AgentReviews\Hero1AxeAOE_HitboxCleanup_20260528\final_close_packet.md
- Output scope: review of the packet below only.

<review_packet>
# Final Review Packet - Hero 1 Axe AOE Hitbox Cleanup And Handoff

## Working Goal

Align the Hero 1 axe AOE VFX hitbox/debug outline with the visual, clean and organize the VFX process/assets/docs for future agents, and prepare a new-agent handoff for item-stat confirmation and future idol overlay VFX work.

## Output Scope To Greenlight

The final user-facing closeout will claim:

- Hero 1 black AOE now has a logical crescent-band hitbox aligned to the current slash visual.
- The proof target set confirms inside-band hits, inner-hollow miss, behind miss, and outside-radius miss.
- The VFX tree/process docs are organized enough for the next agent to continue without rediscovery.
- The next-agent handoff is prepared for normal item/stat confirmation and future idol overlay VFX design.
- The current AOE visual is not being polished further in this pass.

## Applicable Instructions

- `AGENTS.md`: active goal, live repo first, Claude cross-review, PPF/process fidelity, Unreal-owned capture, Niagara combat VFX rules.
- `Gameplay/GAMEPLAY_AGENTS.md`: combat VFX work routes through `Gameplay/Combat`.
- `Gameplay/Combat/CombatVFXAuthoringProcedure.md`: combat VFX authoring and evidence gates.
- `Gameplay/Combat/Hero1AxeAOESlashMechanismPacket.md`: effect-specific AOE packet and hitbox contract.
- `Reports/AGENTS.md`: report/proof artifacts go under `Reports/Proof`.

## Codex Implementation Summary

Runtime/data changes:

- `Source/T66/Data/T66DataTypes.h`: added `FWeaponData::AoeInnerRadiusRatio`.
- `Scripts/SetupWeaponsDataTable.py`: emits `AoeInnerRadiusRatio=0.54` for `Hero_1_black_aoe`, `0.00` for other rows.
- `Content/Data/Weapons.csv` and `Content/Data/DT_Weapons.uasset`: regenerated/reloaded.
- `Content/Data/CombatVFXBindings.csv`: `Hero1Axe_AOE_Base` now has `BaseVisualRadius=411.4`.
- `Source/T66/Gameplay/T66CombatComponent.h/.cpp`: passes `EffectiveSlashInnerRadius` through the Hero 1 frontal-sector query and production VFX log.
- `Source/T66/Gameplay/T66PlayerController_Overlays.cpp`: hitbox proof targets now include `Primary`, `InsideBandForward`, `InsideBandSide`, `InnerHollow`, `OutsideBehind`, and `OutsideRadius`.
- `Scripts/ValidateCombatVFXProductionBindings.py`: validates the binding visual radius, weapon geometry contract, and source guard fragments.

Docs/handoff changes:

- `Gameplay/Combat/Hero1AxeAOESlashMechanismPacket.md`: added `10.4 Crescent-Band Hitbox Close`.
- `Gameplay/Combat/CombatVFXInfrastructureInventory.md`: added current hitbox/binding infrastructure status and generated-binding caveat.
- `Gameplay/Combat/MASTER_COMBAT.md`: updated Hero 1 AOE logical hitbox description.
- `Gameplay/Combat/pending_issues_Combat.md`: clarified manual selected-frame issue and current corrected proof.
- `Reports/Proof/CombatVFX/Hero1AxeAOE_HitboxCleanup_20260528/CLEANUP_STATUS.md`: current VFX tree, implemented changes, evidence, and loose ends.
- `Reports/Proof/CombatVFX/Hero1AxeAOE_HitboxCleanup_20260528/HANDOFF_NEXT_AGENT.md`: prompt for the next agent to verify normal item/stat route and design idol overlay VFX pipeline.

## Verification Performed

- Build succeeded: `T66Editor Win64 Development`, 2026-05-28.
- Weapons DataTable reload succeeded: `Scripts/SetupWeaponsDataTable.py`.
- Combat VFX binding DataTable reload succeeded: `Scripts/SetupCombatVFXBindingsDataTable.py`.
- Production binding validator succeeded after final doc updates:
  - command: `UnrealEditor-Cmd.exe C:\UE\T66\T66.uproject -run=pythonscript -script=C:\UE\T66\Scripts\ValidateCombatVFXProductionBindings.py -unattended -nop4 -nosplash`
  - result: `Success - 0 error(s), 3 warning(s)`
  - warnings were the existing scalability and ToonStyle material include warnings.
- Gameplay capture succeeded:
  - video: `Saved/VideoCaptures/Hero1AxeAOE_HitboxCleanup_20260528_Final_20260528_012913/Hero1AxeAOE_HitboxCleanup.mp4`
  - contact sheet: `Saved/VideoCaptures/Hero1AxeAOE_HitboxCleanup_20260528_Final_20260528_012913/evidence/contact_sheet.png`
  - manifest: `Saved/VideoCaptures/Hero1AxeAOE_HitboxCleanup_20260528_Final_20260528_012913/evidence/manifest.json`
  - visibility checklist: `Saved/VideoCaptures/Hero1AxeAOE_HitboxCleanup_20260528_Final_20260528_012913/evidence/visibility_checklist.md`
  - runtime log: `Saved/VideoCaptures/Hero1AxeAOE_HitboxCleanup_20260528_Final_20260528_012913/T66.log`

Important proof log lines from the final capture:

- `CombatVFXProductionSpawned ... EffectiveSlashRadius=437.52 EffectiveSlashInnerRadius=236.26 AoeInnerRadiusRatio=0.540 BaseVisualRadius=411.40 ... VisualScale=1.063 ... EffectiveDamagePerShot=28`
- `Target=Primary ExpectedHit=1 ActualHit=1 ... Result=PASS`
- `Target=InsideBandForward ExpectedHit=1 ActualHit=1 ... Result=PASS`
- `Target=InsideBandSide ExpectedHit=1 ActualHit=1 ... Result=PASS`
- `Target=InnerHollow ExpectedHit=0 ActualHit=0 ... Result=PASS`
- `Target=OutsideBehind ExpectedHit=0 ActualHit=0 ... Result=PASS`
- `Target=OutsideRadius ExpectedHit=0 ActualHit=0 ... Result=PASS`

Manual visual evidence:

- The final contact sheet frame 45 shows the active red/blue slash inside the red crescent-band debug outline, including the hollow center.
- Visibility checklist is marked `PASS`.

## PPF Close

```text
PPF CLOSE
Process used: AGENTS.md Niagara combat VFX + Hero1AxeAOESlashMechanismPacket hitbox authority process.
Matches declared process: YES
Evidence: Damage authority remains UT66CombatComponent logical query; Niagara remains presentation-only; Unreal-owned capture and logs prove the current crescent-band hitbox contract.
```

## Mechanism Close For This Pass

```text
MECHANISM CLOSE
Mechanism: Logical crescent-band hitbox
Status: PRESENT
Evidence: AoeInnerRadiusRatio=0.54, EffectiveSlashInnerRadius=236.26, target PASS rows for inside band/hollow/behind/outside.
Discriminator test: A filled sector would hit InnerHollow; final log reports InnerHollow ExpectedHit=0 ActualHit=0 Result=PASS.
Reported status: FULL for this hitbox-alignment pass.
```

## Known Caveats

- This is not a renewed final visual-polish pass.
- Normal player-facing item acquisition still needs a next-agent proof. Existing proof-item grants are automation-only.
- Future idol overlays need a reviewed binding/effect-packet design; they are not implemented here.
- `Content/Data/CombatVFXBindings.csv` and `Content/Data/DT_CombatVFXBindings.uasset` are live generated binding files, but `git ls-files` returns no entries for them in this repo state.
- A failed rerun occurred because `BuildT66VideoEvidenceBundle.py` accepts only `start`, `mid`, `impact`, and `dissipate` selected-frame labels. This was corrected by rerunning the capture with `start=43,mid=45,impact=47,dissipate=49`.

## Reviewer Request

Review this final closeout packet as read-only. Identify blocker/major issues only if the final user-facing claims above are unsupported, contradict repo instructions, skip required evidence, overstate completion, or leave the next-agent handoff misleading. First non-empty line must be exactly one of:

`Verdict: APPROVE`

`Verdict: REVISE`

`Verdict: BLOCK`

</review_packet>
