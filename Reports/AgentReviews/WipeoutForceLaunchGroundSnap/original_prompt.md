User request:

Again were having the problem where im under neath the ground, also it seems like im dragged back to the original location. So look into this, I've had somehow got re-enabled or something, but I'm both getting under the ground and I'm having the issue of going back to the location, and the... The way the physics is working right now, when I get hit by the tool, it should push me out of the radius of the tool. Like you should, you know, we're focusing a bit too much on the ragdoll, like rigging and skeletal aspect of it. It should also be more force driven and really like throw me out until I hit a wall, and then I should bounce off the wall. That's what I want next because right now I spawn in and I just get stuck in this little loop. So we need to fix going underneath the ground, we need to fix going back to the location, and then we need to make sure that the distance I'm thrown is much more than it is now.

Task contract:

Working task:
Operator: Codex
Validator: Claude
Scope: fix the current TestRoom wipeout-arm behavior where the hero can sink below the floor, snap/return near the original location, and fail to be launched far enough out of the arm radius.
Stop condition: root cause is identified and patched in runtime code, focused build and staged standalone proof run, and any remaining manual feel-tuning caveats are explicit.

Relevant repo/process rules:
- C:\UE\T66\AGENTS.md requires Codex operator / Claude validator loop for T66 prompts.
- Current operator state is Codex operator, Claude validator.
- Runtime gameplay changes need focused compile verification and staged standalone validation when they affect the playable standalone.
- Existing T66 pure-Chaos hero ragdoll direction should be preserved. Do not re-enable PAC for heroes.

Live-code observations before edit:
- `Source/T66/Gameplay/GameMode/T66GameMode_TestRoom.cpp` currently builds wipeout launch from `MotionDirection` only; that is tangential arm motion, not a radial ejection away from the hub.
- TestRoom wipeout uses a short active/recovery profile (`Incap=0.15`, `MaxRagdoll=0.40`, `BlendOut=0.10`), so repeated hits can happen quickly if the hero remains inside the arm radius.
- `Source/T66/Gameplay/T66KnockbackComponent.cpp` restores control at `RestoreFromKnockback()` and reattaches the mesh after detached ragdoll.
