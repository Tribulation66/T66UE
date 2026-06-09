# Original User Request

Okay, so actually, okay, so that's fine for stage one. Regarding stage two, indeed, Hero1 chat is the MVP, and the idea here is we're already implementing this into the game. So we have the test room where we have a trap there, but this is not supposed to just be like a test room thing, like how the previous files were. This is the game and the test room just happens to have one of these trapped obstacles that serves the function of us, you know, testing. Now, the friend swap raw humanoid rigging instructions were wrong, okay? They were still using the concepts and philosophy that we were using from the original Toon style, which was our previous framework. What you and Clyde need to figure out is like, okay, given this is the direction we're gonna go with the physics, given that the character should feel like Fall Guys characters, how should the rigging and animations be done? Both of you need to have asked this question and figure it out. Then once you have this answer, then you go ahead and you do the rigging, and then you add the animations. You have the idle, the walk, remove the run from it, walk, jump, and leap. Okay, let's keep it simple. We don't even need to put forward leap. And yeah, if you find that get-up poses are good, do those too. And with this whole setup, then you wire it, okay? And you set it as the model for Hero1. Okay, so that's kind of the first step even before, I guess, we build your original Stage 2, which is the Hero Active ragdoll, because those are like physics components. So we need to redo the roadmap, push Hero Active Ragdoll MVP to 3, and everything else down 1, and 2 becomes figure out the rigging and animation solution for the game. And then you're going to replace the content that's currently in the rigging and animation files in the Francois folder, and this new approach that you and Claude will come up with will be our new standard approach that we do for future heroes. Okay, but we need a really good answer regarding this system, and then step 2, and then after we have a character that's rigged and animated and works, we'll move on to the original Stage 2, which is add Hero physics components, keep capsule, mesh is always simulatable, build the pelvis, add pack, etc. Okay, plus the hip constraint components. So those are the steps we're going to do. So now, figure out the solution for the rigging and animation, let me know, and I'll give you the go-ahead if it's good.

# Task Contract

Working task:
Operator: Codex
Validator: Claude
Scope: Re-plan the roadmap so the next stage is a rigging/animation standard for Fall Guys-like heroes, with Hero 1 Chad as the MVP, before the active-ragdoll runtime component stage. This turn is analysis/planning only: no Blender, Unreal import, code edits, or asset mutation unless the user explicitly approves later.
Stop condition: Codex and Claude produce a repo-grounded rigging/animation solution, stale assumptions are corrected, and Codex reports the proposed Stage 2 plan plus what needs user approval before implementation.

# Repo Context Already Observed By Codex

- `.t66/operator-state.json` selects Codex as Operator and Claude as Validator.
- No `ANTHROPIC_API_KEY` was set in Process, User, or Machine scope.
- Current Stage 1 physics docs exist under `Gameplay/Physics`.
- `Gameplay/Physics/MASTER_PHYSICS.md` currently says Stage 2 is active-ragdoll MVP; the user now wants that pushed to Stage 3.
- Current `Model Generation/Instructions/13_FRIENDSLOP_RAW_HUMANOID_RIGGING_INSTRUCTIONS.md` is not accepted as the answer because user says its philosophy is still contaminated by old ToonStyle/legacy humanoid assumptions.
- Existing Hero 1 Chad rigging report exists at `Model Generation/Runs/Pixal3D/FriendSlopProbe_Hero1Male_20260604_1415/Blender/Rigging/Hero_1_Chad_Male_FriendSlop_Rig_Report.md`. It has useful structure, but calls itself a deterministic spike rig, not a hand-polished production animation rig.
- Existing animation-source manifest exists at `.../Blender/Rigging/AnimationSources/friendslop_raw_humanoid_animation_sources_manifest.json` and says clips are Quaternius-derived T66 gameplay roles: Idle, Walk, Jump, Roll. The user wants the future standard to be Idle, Walk, Jump, Leap, plus get-up poses only if useful.
- `Content/Data/CharacterVisuals.csv` has `Hero_1_Chad` using the FriendSlop raw skeletal mesh and Walk/Idle/Jump/Roll animations. The CSV/runtime data shape has no Leap field yet.
- Runtime code still has `RollAnimation`, `RollForward()`, `TryRollForward()`, and single-node animation playback. Replacing roll with leap requires a later runtime/data wiring pass, not just new FBX clips.

# Question For Claude

Produce an independent, repo-grounded answer to the user:

1. What should the new T66 hero rigging and animation standard be if the desired feel is Fall Guys-like, physics-first, and obstacle-driven?
2. What is wrong with simply using the current FriendSlop raw humanoid rigging doc and existing Hero 1 Chad rig/animation outputs as the standard?
3. How should the roadmap be revised: Stage 2 rigging/animation standard and Hero 1 Chad implementation; Stage 3 active-ragdoll physics component; later stages shifted down?
4. What exact documents/files should be replaced or updated after user approval?
5. What should be in/out of scope for the next implementation pass?

Do not suggest mutating files in your answer. This is a planning/analysis answer only.
