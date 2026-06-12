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
- Packet path: C:\UE\T66\Reports\AgentReviews\CombatVFXWaterIdolImpactImplementation\completion_packet.md
- Output scope: review of the packet below only.

<review_packet>
# Combat VFX Water Idol Impact Structure Completion Packet

## Working Goal

Plan and, after review, implement the all-weapons/all-idols impact-source infrastructure in T66 so idols have their own damage source and impact point, with Water idol tested via a simple blue-sphere placeholder triggered from the Hero 1 AOE slash impact, without final Niagara authoring yet.

## Review History

- Implementation plan packet: `C:\UE\T66\Reports\AgentReviews\CombatVFXWaterIdolImpactImplementation\plan_packet.md`
- Claude implementation approval: `C:\UE\T66\Reports\AgentReviews\CombatVFXWaterIdolImpactImplementation\20260528T095423-pass3\claude_review_pass3.md`
- First non-empty review line: `Verdict: APPROVE`

## Implemented Scope

- Added `FT66CombatImpactContext` to `UT66CombatComponent` as the common impact authority seam for weapon and idol damage sources.
- Weapon auto-attacks now publish weapon-owned impact contexts for Pierce, AOE, Bounce, and DOT, including source ID, attack category, impact point, forward vector, radius/shape fields, effective damage, and hit target handles.
- Water idol is the first idol-context consumer:
  - The weapon impact point triggers an idol-owned context.
  - `SourceType=IdolModifier`, `SourceID=Idol_Water`, and `ParentSourceID=<weapon source>` are logged.
  - Water performs its own target query around its own impact point using `FIdolData::AoeRadius`.
  - Water applies damage under `SourceID=Idol_Water` from the new idol impact-context branch at `C:\UE\T66\Source\T66\Gameplay\T66CombatComponent.cpp:2628`.
  - Water owns its own impact point for future chaining, but chaining is not implemented yet.
- Added a Water-only blue sphere placeholder at the idol impact point, used only because final Water Niagara authoring is deferred.
- Added binding lookup/spawn helper for future idol-owned Niagara bindings.
- Suppressed only Water's old visual-only idol projectile lane when the new impact presentation is active.
- Left other idols on the legacy payload path for now, with an Earth neutral proof confirming no Water-only placeholder/impact path leakage. The infrastructure is general; Water is the only wired idol consumer in this pass.
- Added proof automation mode `hero1axeaoewateridolimpact` and script `C:\UE\T66\Scripts\RunHero1AxeAOEWaterIdolImpactProof.ps1`.
- Updated combat docs and logged the out-of-scope runtime timing issue for authored `FIdolData::AoeDelay`.

## Files Changed

- `C:\UE\T66\Source\T66\Gameplay\T66CombatComponent.h`
- `C:\UE\T66\Source\T66\Gameplay\T66CombatComponent.cpp`
- `C:\UE\T66\Source\T66\Gameplay\T66CombatVFX.cpp`
- `C:\UE\T66\Source\T66\Gameplay\T66PlayerController_Overlays.cpp`
- `C:\UE\T66\Scripts\CaptureT66GameplayVideo.ps1`
- `C:\UE\T66\Scripts\RunHero1AxeAOEWaterIdolImpactProof.ps1`
- `C:\UE\T66\Gameplay\Combat\CombatVFXIdolOverlayArchitecture.md`
- `C:\UE\T66\Gameplay\Combat\MASTER_COMBAT.md`
- `C:\UE\T66\Gameplay\Combat\pending_issues_Combat.md`

Known caveat: `T66PlayerController_Overlays.cpp` and `MASTER_COMBAT.md` had pre-existing unrelated working-tree changes. The implementation was integrated without reverting them.

Diff-scoping clarification:

- Task-owned hunks in `T66PlayerController_Overlays.cpp`: `Core/T66IdolManagerSubsystem.h` include; `hero1axeaoewateridolimpact` mode routing; `-T66Hero1AxeAOEProofIdol` handling; proof idol state snapshot/restore; Water/Earth proof target layouts; proof HP override; `DamageBySource` proof logging; stricter Water/Earth target assertions.
- Pre-existing unrelated hunks in `T66PlayerController_Overlays.cpp`: `UT66ProjectileManagerSubsystem`, ranged smoke, boss projectile manager smoke, and non-director route attribution work. These are not claimed as part of this Water idol structure change.
- Task-owned hunks in `MASTER_COMBAT.md`: the `FT66CombatImpactContext` weapon-impact note and the `Idol_Water` first-consumer/blue-sphere placeholder note.
- Pre-existing unrelated hunks in `MASTER_COMBAT.md`: boss projectile manager/projectile routing notes and older Hero 1 AOE hitbox prose outside the impact-context/Water-idol bullets.

## Verification Evidence

- C++ editor build:
  - Command: `& 'C:\Program Files\Epic Games\UE_5.7\Engine\Build\BatchFiles\Build.bat' T66Editor Win64 Development -Project='C:\UE\T66\T66.uproject' -WaitMutex`
  - Result: success.
  - Known pre-existing warning on first full build: `T66Hero1AxeAOEVFXLabActor.cpp(353,3): warning C4996: FNiagaraEmitterInstance::IsReadyToRun`; this is already tracked in `C:\UE\T66\Source\T66\Gameplay\pending_issues_Gameplay.md`.
- Production VFX binding validator:
  - Command: `& 'C:\Program Files\Epic Games\UE_5.7\Engine\Binaries\Win64\UnrealEditor-Cmd.exe' 'C:\UE\T66\T66.uproject' -run=pythonscript -script='C:\UE\T66\Scripts\ValidateCombatVFXProductionBindings.py' -unattended -nop4 -nosplash`
  - Result: exit 0.
  - Non-blocking existing warnings: `r.Upscale.Quality` priority warning and ToonStyle include material warning.
- Water/Earth proof wrapper:
  - Command: `& .\Scripts\RunHero1AxeAOEWaterIdolImpactProof.ps1 -OutputRoot 'C:\UE\T66\Saved\VideoCaptures\Hero1AxeAOE_WaterIdolImpactProof_20260528_104400'`
  - Result: exit 0.
  - Summary: `C:\UE\T66\Saved\VideoCaptures\Hero1AxeAOE_WaterIdolImpactProof_20260528_104400\Hero1AxeAOEWaterIdolImpactProofSummary.md`
  - Water video: `C:\UE\T66\Saved\VideoCaptures\Hero1AxeAOE_WaterIdolImpactProof_20260528_104400\WaterImpact\WaterImpact.mp4`
  - Water contact sheet: `C:\UE\T66\Saved\VideoCaptures\Hero1AxeAOE_WaterIdolImpactProof_20260528_104400\WaterImpact\evidence\contact_sheet.png`
  - Earth video: `C:\UE\T66\Saved\VideoCaptures\Hero1AxeAOE_WaterIdolImpactProof_20260528_104400\EarthNeutral\EarthNeutral.mp4`
  - Earth contact sheet: `C:\UE\T66\Saved\VideoCaptures\Hero1AxeAOE_WaterIdolImpactProof_20260528_104400\EarthNeutral\evidence\contact_sheet.png`
  - Earth neutral uses the same proof mode, Hero 1 black AOE weapon selection, spawned target layout, and auto-attack branch as Water. The only proof input changed is `-T66Hero1AxeAOEProofIdol=Idol_Earth`, making the absence of Water-only impact logs meaningful.
  - The proof wrapper now fails on any `Result=FAIL`; the final run checked 1 Water forbidden pattern and 4 Earth forbidden patterns.
- Staged standalone:
  - Command: `& .\Scripts\StageStandaloneBuild.ps1`
  - Result: `BUILD SUCCESSFUL`.
  - Staged exe: `C:\UE\T66\Saved\StagedBuilds\Windows\T66\Binaries\Win64\T66.exe`
  - Repo shortcut target verified: `C:\UE\T66\T66 Standalone.lnk` -> staged exe.
  - Pinned taskbar shortcut target verified: `C:\Users\DoPra\AppData\Roaming\Microsoft\Internet Explorer\Quick Launch\User Pinned\TaskBar\T66 Standalone.lnk` -> staged exe.
- Staged launch smoke:
  - Command: `& .\Scripts\CaptureT66UIScreen.ps1 -Screen MainMenu -Output 'C:\UE\T66\Saved\VideoCaptures\Hero1AxeAOE_WaterIdolImpactProof_20260528_104400\StagedStandalone_MainMenu.png' -ResX 1280 -ResY 720 -DelaySeconds 2.5 -TimeoutSeconds 60`
  - Result: screenshot captured.
  - Staged screenshot: `C:\UE\T66\Saved\VideoCaptures\Hero1AxeAOE_WaterIdolImpactProof_20260528_104400\StagedStandalone_MainMenu.png`
  - Staged log checked: `C:\UE\T66\Saved\StagedBuilds\Windows\T66\Saved\Logs\T66.log`
  - Log evidence includes MainMenu override and screenshot request; checked fatal/error patterns had no blocking hits.
  - Staged smoke is a frontend launch check, so the idol combat branch is not executed there. The staged log was searched for `CombatImpactContext` and `Idol_Water` and had no matches. Existing staged warnings were Steam/local-content warnings unrelated to the new idol context. Idol-context runtime warning evidence comes from the editor gameplay proof logs.
- Narrow diff hygiene:
  - Command: `git diff --check -- <task files>`
  - Result: no whitespace errors; normal CRLF warnings only.

## Proof Log Highlights

- Water proof:
  - `CombatImpactContext Phase=WeaponPrimary SourceType=WeaponBase SourceID=Hero_1_black_aoe`
  - `CombatImpactContext Phase=IdolPrimary SourceType=IdolModifier SourceID=Idol_Water ParentSourceID=Hero_1_black_aoe`
  - `CombatIdolWaterImpactResolved ... RadiusSource=FIdolData.AoeRadius AoeDelay=0.150 DelayApplied=false Reason=LegacyImmediatePreserved`
  - `CombatVFXIdolImpactBindingLookup SourceType=IdolModifier SourceID=Idol_Water ... Result=None`
  - `CombatVFXIdolImpactPlaceholderSpawned ... Placeholder=BlueSphere VisualScale=1.500`
  - `Target=WaterOnlyInnerHollow ExpectedHit=1 ActualHit=1 ... Result=PASS`
  - `DamageBySource SourceID=Idol_Water TotalDamage=32`
- Earth neutral proof:
  - `CombatImpactContext Phase=WeaponPrimary SourceType=WeaponBase SourceID=Hero_1_black_aoe`
  - `Target=InnerHollow ExpectedHit=0 ActualHit=0 ... Result=PASS`
  - `DamageBySource SourceID=Idol_Earth TotalDamage=36`
  - Forbidden Water-only patterns were absent: `Result=FAIL`, `CombatImpactContext Phase=IdolPrimary SourceType=IdolModifier SourceID=Idol_Water`, `CombatIdolWaterImpactResolved`, `CombatVFXIdolImpactPlaceholderSpawned`.

## PPF Close To Present

Process used: `Gameplay\Combat\CombatVFXAuthoringProcedure.md`, `Gameplay\Combat\VFX_PROCESS_INDEX.md`, and `Gameplay\Combat\CombatVFXIdolOverlayArchitecture.md`.

Matches declared process: YES for the requested structure/proof pass. Final Water Niagara authoring is intentionally deferred; the blue sphere is a temporary structural proof, not accepted as final idol VFX.

Evidence: build success, binding validator, Unreal-owned Water/Earth captures, staged build, staged shortcut verification, and staged smoke screenshot.

## Mechanism Close To Present

- Weapon impact publication: PRESENT. Evidence: weapon impact context log for Hero 1 black AOE and implementation branches for Pierce/AOE/Bounce/DOT.
- Idol-owned damage source: PRESENT for Water. Evidence: `SourceType=IdolModifier`, `SourceID=Idol_Water`, separate target query, `ApplyDamageToTargetHandle` from the Water impact-context branch, and separate `DamageBySource`.
- Idol-owned impact point: PRESENT for Water. Evidence: idol context impact point and blue sphere spawned at that point.
- Future Niagara binding seam: PRESENT as a compiled and reachable seam, not as a real-asset spawn proof. Evidence: idol binding lookup/spawn helper exists and currently falls through to placeholder because no Water idol Niagara is authored yet.
- Placeholder gating: PRESENT. Evidence: only `UsesImpactPresentationForIdol` enters this path, and that helper is currently restricted to `Idol_Water` plus AOE; other idols remain legacy, with Earth neutral proof confirming no Water-only placeholder or Water impact logs.
- Final Water Niagara: DEFERRED by request.

## Requested Final Output Scope

Codex should present a concise final completion summary stating that the structure is implemented, Water was proven with the blue sphere placeholder, Earth neutral proved no leakage, staged standalone was refreshed and smoke-checked, and final Water Niagara remains the next content-authoring step.

</review_packet>
