# Chat 4 Meta/UI Implementation Plan Decision Gate

## Current blocker

The proposed full implementation depends on the Foundation Mob Loot collection API, including target discovery/reservation and collection calls such as `CollectMobLootAt(...)`.

Live repo inspection found no Mob Loot collection or reservation API in `Source/T66`, `Content/Data`, or `Config`. The existing restructuring report also says pet implementation should start after Mob Loot representation is concretely defined.

## Why this blocks full implementation

The plan explicitly says pet collection must consume the Foundation API and must not reimplement collection or touch loot bags. Without the Foundation API, Codex cannot implement or validate the pet's Mob Loot collection behavior, and cannot run the requested Mob Loot collection smoke.

## Choices

1. Wait for Foundation to land the Mob Loot API, then run the full Chat 4 implementation.
2. Approve a split pass now: implement hero selection UI/video infrastructure, pet selection/save/capture shell, and one-active-pet follow behavior, but leave Mob Loot targeting/collection disabled behind a Foundation API seam until the API exists.
3. Narrow this pass to hero selection only and defer all pet runtime/save/boss-capture work until Foundation and Combat seams are both present.

## Current recommended path

Option 1 if the goal is full end-to-end pet behavior and staged runtime proof.

Option 2 only if a partial, clearly gated infrastructure pass is acceptable.
