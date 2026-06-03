# Decision Block: Demo Gating Visibility

## Status

NEEDS_HUMAN_DECISION

## Blocking Decision

The requested "full list of what's deprecated" appears to intersect the repo's
deprecated feature settings for arcade/minigame-related systems. Root
`AGENTS.md` excludes Mini/minigame systems by default unless the user explicitly
names or allows that scope.

## Decision Needed

Should the deprecated inventory include Mini/minigame/arcade deprecated items as
documentation-only inventory entries, without inspecting or changing Mini-owned
runtime code?

## Recommended Path

Allow documentation-only inclusion of centrally declared deprecated
Mini/minigame/arcade items, while keeping all Mini/minigame runtime code,
assets, captures, and implementation changes out of scope for this pass.

## Work Waiting On This Decision

- Phase 1: move drugs, diploma upgrades, and achievements to available content.
- Phase 2: write separate demo-gated-invisible and deprecated-content docs.
- Phase 3: hide remaining visible demo-gated UI entries instead of showing
  COMING SOON.
- Phase 4: compile/capture/stage verification.
