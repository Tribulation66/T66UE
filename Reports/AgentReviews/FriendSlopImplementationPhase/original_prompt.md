User request:
Ok sounds good, yes retarget the animation to the static mesh and just make that the default hero 1, and have the test room just use the selected hero from hero selection no need to override anymore. 2. Sounds good. 3. Sounds good. Go ahead and do this phase of implementation.

Task contract:
Working task:
Operator: Codex
Validator: Claude
Scope: implement this phase: make FriendSlop skeletal Hero 1 Chad the default selected/runtime hero with compatible walk animation, remove the test-room-only skeletal override path so Test Room uses selected hero visuals, make the camera follow ragdoll throws, and replace the current loose generated ragdoll behavior with a controlled/limited profile.
Stop condition: source/content changes are made, imported/reloaded where needed, and verification is attempted with exact results reported.

Repo rules: follow AGENTS.md, OPERATOR_VALIDATOR_PROTOCOL.md, Model Generation/Rigging instructions, data import/reload rules, and staged standalone verification if playable standalone content is affected.
