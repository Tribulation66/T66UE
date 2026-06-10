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

Phase 2/3 interleaved (~22:00): C++ runtime COMPILED CLEAN into editor DLL
(pawn + motor system + scenario + 3 hooks). Blender pipeline v3 running
(v1 failed: bone-heat produced zero weights on the multi-shell mesh →
replaced with numpy distance-based skinning; v2 failed: glTF quaternion
rotation mode made the facing flip a silent no-op + coat flare poisoned hip
width + cuffs faked hand tips → v3 fixes all three). Next: inspect v3 proofs,
UE import, test-room smoke capture.

Version 1.2 landed mid-run (tree now clean; commit c8da91343) — additive
lane commits are safe. Repo git config HIDES untracked files: use
`git status -uall` and explicit `git add <paths>`.

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
