Task: Stage 2 physics-first Hero 1 Chad rig/animation implementation from a clean raw-source foundation.

User constraint followed:
- Ignored other-agent mid-change assets and did not restore old assets.
- Built the Stage 2 path from the raw source GLB at `Model Generation/Runs/Pixal3D/FriendSlopProbe_Hero1Male_20260604_1415/Outputs/Hero_1_Chad_Male.glb`.
- Kept Stage 3 active ragdoll/PAC/hip-constraint runtime work out of scope.

Implementation summary:
- Added `Model Generation/Rigging and Animation/Tools/create_physics_first_hero1_chad_sources.py`.
- Generated fresh Blender source and FBXs under `Model Generation/Runs/Pixal3D/FriendSlopProbe_Hero1Male_20260604_1415/Blender/PhysicsFirstHero/`.
- Produced fresh skeletal FBX plus Idle, Walk, Jump, Leap, RecoverStand, GetUp_Back, and GetUp_Front animation FBXs.
- Added `Model Generation/Rigging and Animation/Tools/import_physics_first_hero1_chad_to_unreal.py`.
- Imported Hero 1 PhysicsFirst assets under `/Game/Characters/Heroes/Hero_1/Chad/FriendSlopRaw/PhysicsFirst`.
- Created Stage 2 physics asset seed `/Game/Characters/Heroes/Hero_1/Chad/FriendSlopRaw/PhysicsFirst/PA_Hero_1_Chad_PhysicsFirst_Stage2Seed`.
- Replaced the standard raw humanoid rigging instructions with the new physics-first raw FriendSlop approach.
- Updated physics, movement, and rigging docs so Stage 2 is rig/animation foundation and Stage 3 is active ragdoll.
- Renamed active movement data/code from Roll to Leap, preserving deprecated Roll/Dash wrappers only as compatibility aliases.
- Updated `Content/Data/CharacterVisuals.csv` header to `LeapAnimation`; Hero_1_Chad now points to the fresh PhysicsFirst mesh and Idle/Walk/Jump/Leap clips.
- Changed Leap runtime movement from a flat renamed dash to a forward-up launch using `LeapStrength` and new `LeapUpwardStrength`.
- Added HeroMovementQA proof logging for Idle/Walk/Jump/Leap clip resolution and Leap launch velocity.

Verification performed:
- Blender generation PASS; QA report says `total_vertices=163496`, `unweighted_vertices=0`, `max_influences_per_vertex=1`, all required bones present, no zero-vertex deform bones.
- Unreal import report PASS at `Reports/AgentReviews/FallGuysHeroRiggingStage2Implementation/physics_first_hero1_unreal_import_report.json`; imported bounds size `[90.2032,52.4361,180]`.
- PhysicsAsset commandlet PASS at `Reports/AgentReviews/FallGuysHeroRiggingStage2Implementation/physics_first_hero1_physics_asset_report.json`; 18 bodies and 17 constraints.
- Focused compile PASS: `Build.bat T66Editor Win64 Development -Project="C:\UE\T66\T66.uproject" -WaitMutex`.
- DataTable reload PASS: `Saved/Logs/T66.log` contains `DT_CharacterVisuals reloaded and saved`; `Content/Data/DT_CharacterVisuals.uasset` timestamp updated.
- HeroMovementQA final capture PASS: `Reports/AgentReviews/FallGuysHeroRiggingStage2Implementation/hero1_chad_physicsfirst_leapqa_final.mp4`, 1280x720, 72 frames, 4.8 seconds.
- Final HeroMovementQA log evidence:
  - Mesh resolved: `/Game/Characters/Heroes/Hero_1/Chad/FriendSlopRaw/PhysicsFirst/SK_Hero_1_Chad_PhysicsFirst.SK_Hero_1_Chad_PhysicsFirst Loaded=YES`.
  - Clips resolved: `HeroIdle=AM_Hero_1_Chad_PhysicsFirst_Idle`, `HeroWalk=AM_Hero_1_Chad_PhysicsFirst_Walk`, `HeroJump=AM_Hero_1_Chad_PhysicsFirst_Jump`, `HeroLeap=AM_Hero_1_Chad_PhysicsFirst_Leap`.
  - Leap launch: `velocity=V(X=3200.00, Z=880.00)`, `cooldown=0.70`.
- Final standalone stage PASS: `Scripts/StageStandaloneBuild.ps1`.
- Shortcut check PASS:
  - `C:\UE\T66\T66 Standalone.lnk` -> `C:\UE\T66\Saved\StagedBuilds\Windows\T66\Binaries\Win64\T66.exe`.
  - taskbar pinned shortcut -> same target.

Known caveats:
- Stage 3 active ragdoll is not implemented here by design.
- The PhysicsAsset is a Stage 2 seed, not final active-ragdoll tuning.
- Old Roll/Dash wrappers remain intentionally as compatibility aliases while input/data migration settles.
- Leap audio still calls `Hero.Movement.Dash` because no `Hero.Movement.Leap` audio event exists yet.
- Existing unrelated warning remains: `FNiagaraEmitterInstance::IsReadyToRun` deprecation in `T66Hero1AxeAOEVFXLabActor.cpp`.
