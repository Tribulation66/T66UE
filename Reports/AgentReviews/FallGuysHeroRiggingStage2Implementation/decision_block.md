# Decision Block - Hero 1 Chad Stage 2 Content State

## Current Task

Implement Stage 2 physics-first Hero 1 Chad rigging and animation foundation.

## Blocking Decision

Before Codex imports or wires new Hero 1 Chad assets, the user needs to confirm the intent of the current content state.

## Evidence

Narrow `git status --short` against Hero 1 Chad content shows old Hero 1 Chad animation/assets are already marked deleted, while other Hero 1 assets are modified:

```text
D Content/Characters/Heroes/Hero_1/Chad/AnimatedToonStyle/AM_Hero_1_Chad_Idle.uasset
D Content/Characters/Heroes/Hero_1/Chad/AnimatedToonStyle/AM_Hero_1_Chad_Jump.uasset
D Content/Characters/Heroes/Hero_1/Chad/AnimatedToonStyle/AM_Hero_1_Chad_Roll.uasset
D Content/Characters/Heroes/Hero_1/Chad/AnimatedToonStyle/AM_Hero_1_Chad_Walk.uasset
M Content/Characters/Heroes/Hero_1/Chad/AnimatedToonStyle/SK_Hero_1_Chad.uasset
```

The current filesystem still contains FriendSlopRaw assets under:

```text
Content/Characters/Heroes/Hero_1/Chad/FriendSlopRaw/
Content/Characters/Heroes/Hero_1/Chad/FriendSlopRaw/Skeletal/
```

## Choices

1. Treat the current content deletion/reorg as intentional.
   - Codex will proceed using the FriendSlopRaw Hero 1 path as the Stage 2 target.
   - Codex will not restore the deleted old AnimatedToonStyle/Pixal3DToonStyle assets.
   - Codex will avoid broad asset cleanup outside Hero 1 Chad Stage 2.

2. Treat the current content deletion/reorg as accidental or unresolved.
   - Codex will pause asset import and runtime wiring until the content state is resolved.
   - Codex can resume after the user or owning process restores/commits/defers the deleted assets.

3. Allow docs/code-only Stage 2 now and defer all asset import/wiring.
   - Codex will update docs and the Roll-to-Leap code/data standard only.
   - The Hero 1 Chad Blender/Unreal asset work remains blocked until the content state is confirmed.

## Recommended Choice

Choice 1 if the old AnimatedToonStyle/Pixal3DToonStyle deletion was part of the current content cleanup direction. Choice 2 if the deletion was not intentional. Choice 3 only if the user wants partial progress before resolving asset state.
