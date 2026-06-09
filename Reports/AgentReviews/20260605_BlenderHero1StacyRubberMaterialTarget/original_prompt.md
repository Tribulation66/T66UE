# Original Prompt And Task Contract

## Original Prompt

MISSION: Develop the "rubber" material look for the hero character, entirely in Blender. Do not touch Unreal. Output is a locked visual target plus a transferable, documented material recipe. The user clarified to use the female Hero 1 character, the currently used/recently generated female Hero 1 in the FriendSlop/FriendslopStyle direction, and target a Fall Guys-like rubber/vinyl material read. First task: locate and confirm the source asset path with Pablo before doing anything else.

## Working Task

Working task: Locate the intended source asset for the female Hero 1 Pixel3D/FriendSlop-style character and stop for Pablo confirmation before any Blender look-development work.
Operator: Codex
Validator: Claude
Scope: Read-only repo discovery plus a durable decision block. No Blender material creation yet, no Unreal editor/runtime work, no Unreal import, no rigging/jiggle, no Git operation.
Stop condition: Pablo confirms the exact source path to use, or corrects the source path.

## Relevant Repo Rules

- Root `AGENTS.md`: derive a task contract; use current live repo state; read `.t66/operator-state.json`; use Operator/Validator loop when available.
- Root `AGENTS.md`: if only the user can decide how the task proceeds, ask once, save `Reports/AgentReviews/<TaskSlug>/decision_block.md`, and stop.
- Model Generation router: Pixal3D/Blender asset source work routes through `Model Generation`.
- User hard boundary: do not touch Unreal. UE5 reproduction is out of scope.

