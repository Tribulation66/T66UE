# Original Prompt And Task Contract

## Original Prompt

User confirmed to use the existing model for the Blender rubber material mission, but asked to first look into two things:

- How much value can come from changing the source image or Pixal3D parameters compared with Blender material work for the Fall Guys-like rubber/vinyl look?
- Check what model the game currently uses for Hero 1 female. The user has confirmed Hero 1 male uses the correct FriendSlop model, but wants to know whether female is also using FriendSlop or not.

The user reminded that Blender MCP is available, but the immediate request is a repo-grounded check and recommendation. Do not touch Unreal.

## Working Task

Working task: Read-only verification of the current Hero 1 female game visual path and a recommendation on Pixal3D/source-image changes versus Blender material work for the rubber target.
Operator: Codex
Validator: Claude
Scope: Inspect live CSV/code/source assets/manifests as needed. No Unreal editor/runtime, no Blender edits, no generation, no file mutations except report/review artifacts.
Stop condition: Report whether Hero 1 female currently uses a FriendSlop model, and explain whether Pixal3D/source-image changes or Blender material/look-dev has higher expected value for this rubber target.

## Relevant Repo Rules

- Root `AGENTS.md`: live repo state first, task contract controls scope, Operator/Validator loop.
- Root `AGENTS.md`: no Unreal work unless in scope. User explicitly says not to touch Unreal for this material mission.
- Model Generation router owns Pixal3D, source image, Blender QA, generated model cleanup.
- Pixal3D router: Pixal3D is separate from TRELLIS; production imports use manifest-driven ToonStyle only when assets enter playable content. This task is read-only and Blender-target planning, not a production import.

