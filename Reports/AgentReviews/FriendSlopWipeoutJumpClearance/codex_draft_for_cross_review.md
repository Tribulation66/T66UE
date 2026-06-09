Working task:
Operator: Codex
Validator: Claude
Scope: Fix the TestRoom wipeout arm hit test so a visibly cleared jump above the cylinder does not ragdoll the hero.
Stop condition: Root cause is corrected in trap collision logic, focused compile and TestRoom proof are run, staged standalone validation is attempted, and limitations are stated.

Change made:
- In Source/T66/Gameplay/GameMode/T66GameMode_TestRoom.cpp, replaced the old single-sided vertical hit band:
  HeroLocation.Z <= WipeoutArmCenterZ + WipeoutArmRadiusUU + 260
  with a capsule-vs-cylinder vertical interval overlap test.
- The new test uses the hero capsule component location and scaled capsule half-height:
  HeroBottom = HeroLocation.Z - HeroHalfHeight
  HeroTop = HeroLocation.Z + HeroHalfHeight
  ArmBottom = WipeoutArmCenterZ - WipeoutArmRadiusUU
  ArmTop = WipeoutArmCenterZ + WipeoutArmRadiusUU
  hit only when the intervals overlap.
- Added t66.TestRoom.WipeoutArmVerticalHitTolerance default 18 uu, clamped 0..120, so feel can be tuned without restoring the old 260 uu false-hit band.

Why:
- The old code treated capsule-center Z up to 530 uu as hittable while the cylinder top is 270 uu. That means a hero could visually clear the arm and still hit.
- The new code makes a jump clear once the bottom of the capsule is above arm top + tolerance, while still allowing side/body overlap to hit when the capsule actually intersects the arm height.

Verification performed:
- Focused compile passed:
  C:\Program Files\Epic Games\UE_5.7\Engine\Build\BatchFiles\Build.bat T66Editor Win64 Development -Project=C:\UE\T66\T66.uproject -WaitMutex
- Editor TestRoom capture passed:
  Scripts/CaptureT66GameplayVideo.ps1 -CaptureMode testragdollchase ...
  Output: C:\UE\T66\Saved\AgentReviews\FriendSlopWipeoutJumpClearance\testragdoll_after_vertical_gate.mp4
  Frames: C:\UE\T66\Saved\AgentReviews\FriendSlopWipeoutJumpClearance\frames_after_vertical_gate
  Log evidence: Saved\Logs\T66.log contains TestRoom wipeout arm scheduled, T66Knockback skeletal launch, and TestRoom wipeout arm impact routed to hero knockback component after the fix.
- Staged standalone refresh passed:
  Scripts\StageStandaloneBuild.ps1 -ClientConfig Development
  Output: C:\UE\T66\Saved\StagedBuilds\Windows\T66\Binaries\Win64\T66.exe
- Staged packaged smoke passed:
  Screenshot: C:\UE\T66\Saved\Codex\FriendSlopWipeoutJumpClearance\staged_testroom_smoke.png
  Log: C:\UE\T66\Saved\StandaloneLogs\FriendSlopWipeoutJumpClearance_TestRoomSmoke.log
  Log contains DirectEntry Run:TestRoom, TestRoom BeginPlay, wipeout arm scheduled, and knockback impact lines.
  rg Fatal/Error returned no matches in the staged smoke log. Profiling DLL load warnings appeared but are normal non-fatal Windows profiling DLL warnings.
- Shortcut target verification passed:
  C:\UE\T66\T66 Standalone.lnk -> C:\UE\T66\Saved\StagedBuilds\Windows\T66\Binaries\Win64\T66.exe
  User pinned taskbar shortcut -> C:\UE\T66\Saved\StagedBuilds\Windows\T66\Binaries\Win64\T66.exe

Limitations:
- Existing capture automation proves the arm still hits correctly in a non-cleared hit scenario and that packaged TestRoom runs with the fix. It does not perform a timed human jump over the rotating arm. The actual player-feel acceptance is the user's live TestRoom playtest.
- The working file already contains prior TestRoom/ragdoll changes from earlier phases; this pass intentionally only added the vertical tolerance CVar and replaced the vertical-band hit check.
