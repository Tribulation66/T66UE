# MotionRig Overnight Run — State Record

Started: 2026-06-09 ~21:00 local. Agent: Claude (overnight autonomous).

## Task Contract

```text
Working task: Build the MotionRig side lane — a new master rigging+animation
foundation (full physics-driven movement, motor-driven skeleton, Fall Guys
style) proven on Hero 1 Chad male in the Test Room of the staged standalone
build.

Scope:
- NEW: Source/T66/Gameplay/MotionRig/** (pawn, motor system, scenario/telemetry)
- NEW: Content/Characters/MotionRig/Hero_1/** (fresh rig, clips, physics asset)
- NEW: MOTION_RIG.md (authority doc), Model Generation/Instructions/14_MOTIONRIG_*.md
- NEW: Scripts/MotionRig/** (Blender pipeline, capture/contact-sheet harness)
- NEW: Reports/AgentReviews/MotionRig_20260609/** (this record, evidence, packet)
- HOOKS (minimal external edits): T66.uproject (+PhysicsControl plugin — DONE),
  Source/T66/T66.Build.cs (+PhysicsControl module), test-room pawn override in
  GetDefaultPawnClassForController (CVar t66.MotionRig.TestRoom, default 1),
  small input bridge in T66PlayerController movement handlers (interface-based,
  no behavior change when pawn is not a MotionRig pawn).
- DO NOT: edit live hero/gameplay behavior, touch other agents' in-flight files,
  triage/restore the user-owned dirty tree, push to remote.

Stop condition: Hero 1 + Test Room in staged standalone spawns the MotionRig
pawn with physics locomotion (Idle/Walk/Jump/Dive + knockdown/recovery), the
capture harness produces contact sheets + telemetry metrics vs rubric, and the
morning packet exists — OR weekly usage >= 77% — OR an unrecoverable in-lane
blocker (document + clean handoff).
```

## Standing user instructions (this run)

- Multiple agents are active in this repo. On UNRELATED blockers (file locks,
  UBT mutex, editor open): WAIT and retry; do not stop the run.
- 5-hour usage limit: PARK (checkpoint + wakeup chain until reset), never stop.
  Weekly >= ~77%: hard stop with handoff. Endpoint: see
  Reports/AgentReviews/MotionRig_20260609/usage_guard_notes.md or memory.
- Work until done; morning packet is the deliverable.
- Use neutral technical wording in all messages (impact reaction, knockdown,
  recovery) to avoid automated-filter false positives.

## STATUS 2026-06-11 ~00:30: MODEL SWAP — both body types on the new physics models

Replaced the rig source with the simple-clothing physics pair from
HeroChadStacy_SourceAssets_20260609_0536 (Hero2Chad.glb = male,
Hero1Stacy.glb = female) and made the lane body-type aware: the pawn
resolves /Game/Characters/MotionRig/Hero_1_Male or Hero_1_Female from the
hero-select ET66BodyType (Chad/Stacy); old Hero_1 content folder deleted.
Pipeline now parameterized (BuildMotionRig.py --name + --front override —
the toe-direction facing auto-detect is fooled by chunky boots, Pixal3D
sources need --front +y; albedo PNG exported from the GLB and imported as
T_MotionRig_<name>_BaseColor). ImportMotionRig.py + the PA commandlet loop
both characters. New flag -T66BodyType=chad|stacy (direct entry) +
CaptureMotionRig.ps1 -BodyType. Verified: both PAs PASS with the exact unit
guards (98.10/45.90/rootScale 1); male_v1 + female_v1 walkcircle captures
show the correct intact textured model per body type, facing travel.

## (superseded) STATUS 2026-06-10 ~17:30: RENDER BIND FIXED — skin intact (cmbake_v1)

The last open issue (rendered skin stretching/crumpling) is closed. Root
cause chain, proven by raw binary FBX probes (ProbeFbxRaw.py/ProbeFbxRaw2.py):
the Blender FBX exporter converts NOTHING m→cm — a meter-scene export writes
meter numbers everywhere and compensates with scale=100 on the armature/mesh
OBJECT nodes ("keyed channels arrive cm" was a misread of that compensation).
UE imports that as a scale-100 root bone; physics generation and world-space
bone writes ignore root scale → collapsed bodies/anchors and a cm-sized
render. Post-import ref surgery couldn't fix rendering because LOD render
data keeps bind-dependent caches from import.

FIX (deterministic, in-pipeline): (1) BuildMotionRig.py converts ALL data to
real cm at the DATA level (Mesh.transform/Armature.transform x100 + location
fcurves x100 — object-level scale+apply double-scales the parented mesh) and
exports with global_scale=0.01 to cancel the exporter's invariant x100 →
file numbers are cm with node scales 1.0; (2) skeletal FBX carries a baked
2-key bind-pose animation; (3) ImportMotionRig.py forces the LEGACY FBX
importer (Interchange.FeatureFlags.Import.FBX 0 — Interchange IGNORES
FbxImportUI options incl. use_t0_as_ref_pose) and imports with
use_t0_as_ref_pose=True; (4) the commandlet's persistent ref-pose surgery is
DELETED, replaced by a component-space unit guard (refuses pelvisCompZ<50).
Verified: MOTIONRIG_PA_REFPOSE pelvisLocalZ=98.10 calfLocal=45.90
rootScale=1; [MR_SURVEY] bodyZ feet 13 / calves 55 / pelvis 101 / head 150,
real anchor lengths; cmbake_v1 walkcircle frames show a full-size intact
textured character. Dive/impact captures + restage + commit follow.

## (superseded) STATUS ~00:15 (June 10): ACCEPTANCE MET — staged standalone smoke PASS

T66.exe (staged tonight, post cook-fix) boots → Test Room → MotionRig pawn
from cooked assets → motors live → scenario runs → telemetry written. Bean
pitch/roll locked (lean 89°→0°); dive prone via pelvis PD target. Remaining:
feel tuning (foot-skate is the #1 axis), posture polish, punch list in
morning_review.md.

## (superseded) STATUS ~02:45: FOUNDATION WORKING — parked-or-tuning depending on usage

Walk/jump/dive/impact all function and render (textured, FriendSlop master).
Full evidence suite + rubric scores under captures/final_*. morning_review.md
is COMPLETE. Staged standalone build cooking in background (bsh89cwi6) —
smoke-test it on wake if parked. Remaining work is pure CVar tuning vs the
rubric (axes 1-4) + punch list in morning_review.md. v15-v23 chronicle:
detach-the-mesh fixed rendering (old lane's bDetachMeshDuringRagdoll); direct
SLERP drives + one-way pelvis PD are the working motor stack; PhysicsControl
AND PhysicalAnimationComponent both produced zero force on 5.7 (do not retry
without the RigidBodyWithControl node path).

## Phase checklist

- [x] Phase 0a: Recon (usage OK weekly 7%, Blender 5.1 found, PhysicsControl
      plugin present + enabled in uproject, input flow mapped, pawn-class hook
      identified, source GLB verified, prior reports located)
- [ ] Phase 0b: Autopsy notes + design lock (MOTION_RIG.md)
- [ ] Phase 1: Capture/telemetry harness + baseline capture of CURRENT hero
- [ ] Phase 2: Fresh rig + Idle/Walk/Jump/Dive (+GetUp_Front/Back) via Blender
      headless pipeline; UE import
- [ ] Phase 3: Bean pawn + PhysicsControl motor runtime + state profiles
- [ ] Phase 4: Test-room wiring (CVar pawn override + input bridge)
- [ ] Phase 5: Tuning loop vs rubric (telemetry + contact sheets)
- [ ] Phase 6: Morning packet (morning_review.md + evidence)

## Key facts for resume (fill as run progresses)

- Engine: C:\Program Files\Epic Games\UE_5.7 (launcher build)
- Blender: C:\Program Files\Blender Foundation\Blender 5.1\blender.exe
- Source GLB: Model Generation/Runs/Pixal3D/FriendSlopProbe_Hero1Male_20260604_1415/Outputs/Hero_1_Chad_Male.glb
- Pawn class hook: AT66GameMode::GetDefaultPawnClassForController_Implementation
  (T66GameMode_Spawning.cpp:459); override in AT66GameMode_TestRoom.
- Input: legacy axis bindings MoveForward/MoveRight/Jump on AT66PlayerController
  (T66PlayerController_Input.cpp:332+); movement intent flows via
  UpdateHeroMovementIntent() (T66PlayerController_Movement.cpp:32) into
  UT66HeroMovementComponent; Leap input exists (HandleLeapPressed → Hero->Leap()).
- Capture: Scripts/CaptureT66GameplayVideo.ps1 (editor -game, frame PNGs,
  ffmpeg; modes via -CaptureMode; -ExecCmds passthrough; 498 lines).
- Prior Stage 2 rig (cautionary): 23 bones, 163k verts, MAX 1 INFLUENCE/VERTEX
  (hard skinning → rigid joints — fix in new rig: 3-4 influences, smooth falloff),
  180cm height, Blender front -Y → UE +X. Rig QA passed; failure was runtime
  model (kinematic playback + full-limp-only physics), per
  Source/T66/Gameplay/Physics/pending_issues_Physics.md.

## Current step

Phase 4 smoke (~22:00): FIRST IN-GAME CAPTURE RUNNING (walkcircle, side cam).
Done since last checkpoint: Blender v3 PASS (facing flip fixed — glTF imports
are QUATERNION rotation mode, set rotation_mode=XYZ before euler assignment;
legs measured at knee band below coat flare; arm tips walk down cuff column);
UE import PASS via -ExecutePythonScript (-run=pythonscript crashes in
AssetTools: Slate assertion; FBX auto-PA does not run automated) → PA built
by new editor commandlet -run=T66MotionRigPhysicsAsset (18 bodies capsules,
17 constraints, FBX armature-root body culled). Capture harness: reuses
generic screenshot sequencer (-T66GameplayAutoScreenshotSequenceDir/Count/
Interval + frame_%04d.png, auto-quits 1.5s after last frame) + direct entry
(-T66Entry=Run:TestRoom -T66Hero=Hero_1) + scenario self-start
(-T66MotionRigScenario=<name> -T66MotionRigCamera=<cam>).

CHECKPOINT COMMIT: "MotionRig: new physics-first hero motion foundation
(side lane, Phase 0-2)" — 29 files, lane + 4 hooks only.

Usage 22:30: 5h 77% (resets 23:40 — at ~17%/h burn the 90% park threshold
lands ~23:15-23:25; plan a clean checkpoint commit before then, park the
few minutes to reset if needed). Weekly 16% (stop at 77% weekly).

- walkcircle_v3: new DLL confirmed loaded (MR_DIAG strings in binary) but
  pawn timers never fired + no telemetry + black world on chase cam too.
  ROOT CAUSE FOUND: game mode logs "Spawning at PlayerStart: (0,0,0)" — pawns
  spawn at MAP ORIGIN (inside the wipeout-arm hub cube) and the test-room flow
  teleports them AFTERWARDS. v1's tiny body tolerated the interpenetration;
  the full-size v2/v3 body detonates on depenetration before the teleport
  (pawn destroyed → dead view target = black world, controller UI survives).
  FIX: physics bring-up (mesh sim + pelvis constraint + motors) DEFERRED 0.75s
  behind bPhysicsLive gate; bean velocities zeroed at bring-up. Also fixed
  capture script copying STALE telemetry from older runs (freshness check).
- walkcircle_v4: deferred bring-up SURVIVED spawn ([MR_BRINGUP] at test-room
  start, meshSim=1 motors=1) but pawn died between 0.75s and 3s; world black.
  Measured the exported FBX: 18,000 units tall — the x100 bake had stacked on
  the FBX exporter's own m→cm conversion (KAIJU). Reverted the bake; the v1
  "tiny character" read was wrong (camera distance + crumple misled).
- walkcircle_v5: world renders again, scenario ran to completion, telemetry
  real. Character = heap (pelvis==head z≈60), bean hanging at 131, never
  grounded, no walk. TWO causes found: (a) controller axis bindings fire every
  frame with zero and STOMP scripted input → SetScenarioInputOverride; (b)
  motors hold nothing.
- walkcircle_v6: input override works (Idle→Walk→Jump, vx to 62) but body
  still a heap. [MR_DIAG]: meshBodies 18/18 simulating, meshMass=485.3 kg (!)
  — auto-PA capsule masses are ~7x human; motors and the 70kg bean are
  out-muscled. FIX: authored per-bone mass table (70 kg total) applied at
  motor init + motor strengths x8.
- walkcircle_v7: BEAN FIXED — grounded=1, resting z=90, meshMass=70.0
  (authored masses applied). Skeleton still drapes (pelvis 3.2) — PhysicsControl
  controls created but produce NO force despite x8 strengths.
- walkcircle_v8: PIVOTED drive plumbing to UPhysicalAnimationComponent (the
  pre-flight fallback; same architecture, sets + state profiles preserved).
  Pelvis STILL 1.7 — heap is a compact rubble pile, not a body-shaped drape.
  NEW HYPOTHESIS: bodies are DISCONNECTED at runtime (no joint constraints) —
  that would null BOTH drive systems (PAC drives live on the joint
  constraints) and explain the rock-pile heap shape.
- walkcircle_v9: running — MR_DIAG extended with runtime constraint count +
  current anim asset name to confirm.

Known environment facts: ANOTHER AGENT is actively running the cleanup
program (Archive deletions in tree, occasional UnrealEditor DLL locks —
build retries with -WaitMutex + 90s backoff handle it; NEVER chain
commandlet runs off a piped build via && (tail eats the exit code), check
"Result: Succeeded" explicitly).

Version 1.2 landed mid-run (tree clean; c8da91343) — additive lane commits
safe. Repo git config HIDES untracked files: `git status -uall` + explicit
`git add <paths>`.

## Iteration log (Phase 4/5)

- walkcircle_v1: pipeline end-to-end PASS. Character 1/100 scale (Blender
  meters → UE cm; FIX: bake x100 into mesh+armature before FBX export, clips
  are rotation-only so unaffected). Bean frozen (see v2).
- walkcircle_v2: scale fixed, bean STILL frozen at spawn z≈196, never grounded,
  velocity says falling but position barely moves = constraint suspension.
  DIAGNOSIS: PlayAnimation/SetAnimationMode re-initializes articulation and
  clobbers SetSimulatePhysics(true) → mesh bodies kinematic → bean dangles
  from pelvis constraint anchored to kinematic (infinite-mass) bodies.
  FIX: EnsureMeshSimulation() re-asserts SetAllBodiesSimulatePhysics after
  every PlayAnimation + BeginPlay order rework + isolation CVars
  (t66.MotionRig.Debug.EnableMeshSim/EnableConstraint/EnableMotors) +
  [MR_DIAG] one-shot snapshot log at +3s.
- v2 fixed camera showed BLACK world (UI fine) — camera issue parked; next
  captures use -Camera chase until physics is proven, then debug fixed cams.

## Decision log

- D1: No branch. Additive new-lane files committed on current branch (main),
  staging ONLY MotionRig-lane paths + the 4 hook files. Never `git add -A`,
  never reset/checkout/stash (protects other agents' + user's dirty tree).
- D2: MotionRig pawn = fresh APawn subclass (NOT AT66HeroBase) + small
  interface-based controller input bridge. Avoids ACharacter/CMC entanglement.
- D3: New rig is authored fresh in Blender (own spec), not derived from the
  Stage 2 armature; smooth skinning (3-4 influences) replaces hard skinning.
- D4: Mesh plays pose-target clips kinematically ONLY as motor target source
  (PhysicsControl UpdateTargetCaches); all bodies simulate; states are motor
  profiles. No kinematic/simulated mode switch anywhere.
