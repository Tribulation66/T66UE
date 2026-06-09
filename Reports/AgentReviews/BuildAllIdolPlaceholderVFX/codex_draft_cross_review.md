Codex draft for cross-review: Build all temporary idol placeholders plus No Weapon.

Task contract:
- Operator: Codex
- Validator: Claude
- Scope: Implement temporary/basic-shape placeholder VFX for all 20 idols and No Weapon as a white single-target punch ball. Keep this as placeholder infrastructure, not final Niagara polish.
- Stop condition: code/data changes made, compile/build/runtime verification attempted, proof gaps reported.

Implemented:
- `Source/T66/Gameplay/T66CombatVFX.cpp`
  - Added default-on `T66.VFX.ForcePrimitiveIdolPlaceholders`.
  - Added primitive idol placeholder builders for Fire, Ice, Electricity, Nature, and Wind across AOE, Pierce, Bounce, and DOT.
  - Uses Fire red, Ice light blue, Electricity purple, Nature green, Wind gray.
  - Primitive path runs before imported Niagara/Blueprint idol effects while the CVar is enabled.
- `Source/T66/Gameplay/T66CombatComponent.cpp/.h`
  - Passes idol rarity into placeholder VFX so black/red/yellow/white scale/quantity changes apply.
  - Dispatches primitive activation VFX on idol traveler arrival/fallback for AOE, Pierce, Bounce, and DOT.
  - Resolves traveler visual element from normalized idol ID first, including Wind.
- `Source/T66/Gameplay/T66OutgoingTravelerPoolSubsystem.cpp/.h`
  - Expanded known traveler visual profiles from 16 to 20.
  - Mapped all 20 element/category profile IDs onto the four configured temporary primitive mesh slots: AOE sphere, Pierce cone, DOT cylinder, Bounce cube.
- `Source/T66/Gameplay/T66TemporaryProjectileSystem.cpp`
  - Added `HeroSingleTarget`/No Weapon profile as a larger white sphere.
- `Source/T66/Gameplay/T66CombatShared.cpp`
  - Added Wind proof IDs and made SingleTarget category secondary-stat fallback `None`.
- `Scripts/RunOutgoingTravelerVisualProfilesGate.ps1`
  - Updated gate case from 16 to 20 visual profiles.

Verification:
- `Build.bat T66Editor Win64 Development -Project=C:\UE\T66\T66.uproject -WaitMutex -NoHotReloadFromIDE`
  - Passed after final source edit.
- `Scripts\StageStandaloneBuild.ps1`
  - UAT completed successfully after waiting out a background AutomationTool run.
  - Staged exe verified at `C:\UE\T66\Saved\StagedBuilds\Windows\T66\Binaries\Win64\T66.exe`.
  - Both standalone shortcuts target that exe.
- `Scripts\RunOutgoingTravelerVisualProfilesGate.ps1 -Runs 1 -Count 200 -SampleSeconds 1 -ScreenshotSequenceCount 1 -ScreenshotDelaySeconds 6 -PostCaptureScreenshotDelaySeconds 0.5 -TimeoutSeconds 140`
  - Passed from the staged exe.
  - Summary: `C:\UE\T66\Saved\VideoCaptures\OutgoingTravelerVisualProfiles_20260605_103044\visual_profiles_gate_summary.json`.
  - Mixed case used 20 profiles, peak live 200, dropped 0, failed spawns 0.
- Focused Wind AOE capture:
  - `C:\UE\T66\Saved\VideoCaptures\IdolPrimitiveActivation_WindAOE_20260605_101552\WindAOE.mp4`.
  - Log shows `TravelerVisual.Wind.AOE`, `CombatVFXPrimitiveIdolPlaceholderSpawned SourceID=Idol_Wind_AOE Category=AOE Element=Wind Rarity=Black Radius=315.00`, and `T66IdolAOETravelerArrived`.

Known caveats:
- The Wind AOE proof uses the existing Hero1 axe idol-impact harness, whose hitbox expectations are still shaped around the Hero 1 slash/hollow proof. It logs `Result=FAIL` for old slash-exclusion targets because Wind AOE is circular; the VFX routing log is still valid.
- The existing capture camera is not a clean shape-review lab for every idol. This implementation wires the placeholders and gives runtime routing/profile proof; per-idol visual signoff should use a dedicated idol VFX lab/camera next.
- Staged log includes unrelated startup/other-system warnings such as missing profiling DLLs and `NS_MobLootPool` readiness; the outgoing traveler gate did not hit its blocking outgoing-traveler warnings/errors.

Questions for Claude:
- Are there repo-rule or implementation issues that would make this unsafe to report as implemented within the temporary placeholder lane?
- Did Codex miss any obvious wiring path for the 20 idol placeholders or No Weapon white single-target profile?
- Are any caveats above misstated or under-reported?
