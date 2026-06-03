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
- Packet path: C:\UE\T66\Reports\AgentReviews\CombatVFXAoeWaterAlignmentRuntime\completion_packet.md
- Output scope: review of the packet below only.

<review_packet>
# Combat VFX AOE / Water Alignment Completion Packet

## Working Goal

Align the Hero 1 AOE weapon visual and Water idol placeholder with their authoritative damage footprints, then verify the result with the repo Unreal gameplay video capture process.

## Roles And Review

- Operator: Codex in the active workspace.
- Validator: Claude Code CLI, `claude-opus-4-8`, through `Scripts/Invoke-ClaudePlanReview.ps1`.
- Plan review: `Reports/AgentReviews/CombatVFXAoeWaterAlignmentRuntime/20260528T224418-pass1/claude_review_pass1.md`, `Verdict: APPROVE`.
- Delta review: `Reports/AgentReviews/CombatVFXAoeWaterAlignmentRuntime/20260528T225259-pass2/claude_review_pass2.md`, `Verdict: APPROVE`.

## Implemented Scope

Changed runtime/context code:

- `Source/T66/Gameplay/T66CombatComponent.h`
  - Added `DamageCenter` and `bDamageCenterValid` to `FT66CombatImpactContext`.
  - Changed `TrySpawnBoundWeaponBaseSlashVFX` to consume the weapon impact context instead of a loose location/forward pair.

- `Source/T66/Gameplay/T66CombatComponent.cpp`
  - Logs `DamageCenter`, `ImpactPoint`, and validity flags in verbose impact context proof logs.
  - Keeps Hero 1 AOE Niagara spawned from the center-pivoted slash mesh damage center.
  - Logs `VisualPivot`, `VisualAnchorModel=BandAnchored`, and `ImpactOffsetFromDamageCenter` for the Hero 1 AOE weapon VFX.
  - Sets Hero 1 AOE weapon `DamageCenter=SlashCenter` and `ImpactPoint=annulus midpoint`.
  - Sets Water idol `DamageCenter=ImpactPoint` for its own spherical damage source.
  - Gives Water idol debug sphere proof a distinct `Idol Idol_Water AOE Damage` label through the existing debug draw path.

- `Source/T66/Gameplay/T66CombatVFX.cpp`
  - Uses `DamageCenter`/`ImpactPoint` fields in idol impact spawn logs.
  - Changes the temporary Water placeholder from fixed `0.85` scale to `Radius / 50.0`.
  - Spawns the blue sphere at the idol damage center with `VisualRadius=Radius`.

- `Source/T66/Gameplay/T66PlayerController_Overlays.cpp`
  - Updates only the Water proof harness target expectations: the target outside weapon range but inside Water radius is now expected to hit, and a farther `OutsideAllRadius` target proves the Water boundary still misses.

## Verification Performed

1. Focused compile:

```text
Build.bat T66Editor Win64 Development -Project=C:\UE\T66\T66.uproject -WaitMutex -NoHotReloadFromIDE
Result: Succeeded
```

2. First gameplay capture:

- Produced `Saved/VideoCaptures/Hero1AxeAOE_WaterIdolAreaAlignment_20260528/WaterIdolAreaAlignment.mp4`.
- Confirmed runtime alignment logs, but exposed stale proof harness expectation for `OutsideRadius`.

3. Delta compile:

```text
Build.bat T66Editor Win64 Development -Project=C:\UE\T66\T66.uproject -WaitMutex -NoHotReloadFromIDE
Result: Succeeded
```

4. Final gameplay capture:

- Video: `C:\UE\T66\Saved\VideoCaptures\Hero1AxeAOE_WaterIdolAreaAlignment_Final_20260528\WaterIdolAreaAlignment_Final.mp4`
- Contact sheet: `C:\UE\T66\Saved\VideoCaptures\Hero1AxeAOE_WaterIdolAreaAlignment_Final_20260528\evidence\contact_sheet.png`
- Manifest: `C:\UE\T66\Saved\VideoCaptures\Hero1AxeAOE_WaterIdolAreaAlignment_Final_20260528\evidence\manifest.json`
- Selected frames: `C:\UE\T66\Saved\VideoCaptures\Hero1AxeAOE_WaterIdolAreaAlignment_Final_20260528\evidence\selected_frames.md`
- Visibility checklist: `C:\UE\T66\Saved\VideoCaptures\Hero1AxeAOE_WaterIdolAreaAlignment_Final_20260528\evidence\visibility_checklist.md`
- Log excerpt: `C:\UE\T66\Saved\VideoCaptures\Hero1AxeAOE_WaterIdolAreaAlignment_Final_20260528\evidence\water_idol_alignment_log_excerpt.md`
- Copied runtime log: `C:\UE\T66\Saved\VideoCaptures\Hero1AxeAOE_WaterIdolAreaAlignment_Final_20260528\T66.log`
- ffprobe: 1280x720, 12 fps, 108 frames, 9.0 seconds.

5. Staged standalone validation:

```text
powershell -ExecutionPolicy Bypass -File C:\UE\T66\Scripts\StageStandaloneBuild.ps1 -ClientConfig Development
BUILD SUCCESSFUL
Standalone build ready at C:\UE\T66\Saved\StagedBuilds\Windows\T66\Binaries\Win64\T66.exe
Updated C:\UE\T66\T66 Standalone.lnk and pinned taskbar T66 Standalone.lnk to the staged exe.
```

## Final Runtime Evidence

Weapon source:

- `CombatImpactContext Phase=WeaponPrimary SourceID=Hero_1_black_aoe`
- `DamageCenter=V(X=360.00, Z=64.00) DamageCenterValid=1`
- `ImpactPoint=V(X=696.89, Z=64.00) ImpactPointValid=1`
- `Radius=437.52 InnerRadius=236.26 HalfAngleDegrees=90.00`

Weapon visual:

- `CombatVFXProductionSpawned Binding=Hero1Axe_AOE_Base`
- `Location=V(X=360.00, Z=134.00)`
- `VisualPivot=V(X=360.00, Z=64.00)`
- `VisualAnchorModel=BandAnchored`
- `ImpactOffsetFromDamageCenter=336.89`
- `EffectiveSlashRadius=437.52 EffectiveSlashInnerRadius=236.26 BaseVisualRadius=411.40 VisualScale=1.063`

Water idol source:

- `CombatImpactContext Phase=IdolPrimary SourceID=Idol_Water ParentSourceID=Hero_1_black_aoe`
- `DamageCenter=V(X=696.89, Z=64.00) DamageCenterValid=1`
- `ImpactPoint=V(X=696.89, Z=64.00) ImpactPointValid=1`
- `Radius=300.00 HitTargets=5 EffectiveDamage=8`
- `CombatIdolWaterImpactResolved ... Radius=300.00 RadiusSource=FIdolData.AoeRadius ... DamageCenter=V(X=696.89, Z=64.00) ImpactPoint=V(X=696.89, Z=64.00)`

Water placeholder visual:

- `CombatVFXIdolImpactPlaceholderSpawned SourceID=Idol_Water ParentSourceID=Hero_1_black_aoe`
- `VisualAnchor=V(X=696.89, Z=64.00)`
- `VisualLocation=V(X=696.89, Z=64.00)`
- `Radius=300.00 VisualRadius=300.00 Placeholder=BlueSphereAreaRead VisualScale=6.000`

Damage proof:

- `Primary`: PASS
- `WaterOnlyInnerHollow`: PASS
- `WeaponOnlyOuterBand`: PASS
- `InsideBandSide`: PASS
- `WaterOnlyOuterRadius`: PASS
- `OutsideAngleEdge`: PASS
- `OutsideBehind`: PASS
- `OutsideAllRadius`: PASS
- `DamageBySource SourceID=AutoAttack TotalDamage=98`
- `DamageBySource SourceID=Idol_Water TotalDamage=48`

## PPF CLOSE

Process used: `CombatVFXVisualDamageAlignmentContract.md`, existing Hero 1 AOE packet, and `Scripts/CaptureT66GameplayVideo.ps1`.

Matches declared process: YES.

Evidence: code-level damage center/impact point split, runtime logs for weapon and idol contexts, radius-matched Water placeholder log, same-run gameplay video/contact sheet, target pass/fail logs, and staged standalone build success.

## MECHANISM CLOSE

Mechanism: damage-center/impact-point separation for irregular weapon shapes.
Status: PRESENT.
Evidence: WeaponPrimary logs separate `DamageCenter=360` and `ImpactPoint=696.89`.
Discriminator test: Water triggers from band impact point, not the hollow center.
Reported status: FULL for this runtime structure proof.

Mechanism: visual pivot remains tied to authored carrier pivot.
Status: PRESENT.
Evidence: weapon VFX logs `VisualPivot=DamageCenter`, `VisualAnchorModel=BandAnchored`, and production Niagara path.
Discriminator test: no actor-side debug geometry replaces the production slash carrier.
Reported status: FULL for this runtime structure proof.

Mechanism: Water idol area-read placeholder footprint mapping.
Status: PRESENT.
Evidence: placeholder logs `Radius=300.00`, `VisualRadius=300.00`, and `VisualScale=6.000`.
Discriminator test: the old compact-marker lookalike would have logged `VisualRadius` near `42.5`; it no longer does.
Reported status: FULL for temporary proof placeholder; final Water Niagara remains future work.

Mechanism: idol damage source remains independent.
Status: PRESENT.
Evidence: `DamageBySource SourceID=Idol_Water TotalDamage=48`, parent source `Hero_1_black_aoe`, and Water-specific target passes.
Discriminator test: Water-only targets receive Water damage without requiring weapon-sector damage.
Reported status: FULL for this runtime structure proof.

Mechanism: temporal video proof.
Status: PRESENT.
Evidence: 9-second Unreal-owned MP4 plus evidence bundle.
Discriminator test: final proof uses video/contact sheet and log evidence, not a single still or desktop screenshot.
Reported status: FULL for structure proof.

## Caveats

- The blue sphere is intentionally temporary and opaque; it proves Water AOE center/radius, not final art quality.
- Final Water idol Niagara needs its own packet, source/mechanism gate, and visual-polish proof later.
- Existing unrelated uncommitted changes in the worktree were not reverted.

## Requested Final Answer Scope

Report the implemented changes, the validation result, and provide the final MP4 path for Pablo to review.

</review_packet>
