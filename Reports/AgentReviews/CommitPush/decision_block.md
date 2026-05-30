# Commit And Push Decision Block

Working goal: Commit approved current T66 changes to `main`, push `main`, create and push the required version tag, and verify `main` matches `origin/main` with tracked changes classified or clean.

Decision gate saved: 2026-05-30.

## Why This Needs User Decision

The live repo state cannot be safely committed under the root instructions without explicit scope/version decisions:

1. `main` is already one commit ahead of `origin/main`.
2. The working tree has a large tracked dirty set across config, data, docs, scripts, runtime source, LFS-tracked Unreal assets, and deleted tracked files.
3. At least one changed tracked path is Mini/minigame-owned (`Source/T66/UI/Screens/T66MinigamesScreen.cpp`). Root `AGENTS.md` excludes Mini/minigame scope unless the user explicitly includes it.
4. `Config/DefaultGame.ini` currently has `ProjectVersion=alpha-0.8`, and the `alpha-0.8` tag already exists on `origin/main`. Root `AGENTS.md` says "commit and push" includes creating and pushing the next version tag.
5. The tracked set includes LFS-tracked Unreal assets, including modified DataTable `.uasset` files and deleted world/interactable `.uasset` files. This may require a real LFS-inclusive release commit rather than the prior non-LFS-safe pattern.

## Decisions Needed

Please choose or override these defaults:

1. Scope to stage:
   - Default if you say `no clarification needed`: commit all current tracked changes, including Mini/minigame-owned paths and LFS-tracked Unreal assets/deletions.
   - Alternative: commit only non-Mini/non-LFS text/code/docs changes and leave excluded tracked changes explicitly deferred, which means the tree will not be fully clean.

2. Version/tag:
   - Default if you say `no clarification needed`: bump `ProjectVersion` to `alpha-0.9`, commit it, create tag `alpha-0.9`, and push `main` plus `alpha-0.9`.
   - Alternative: provide a different version/tag, or explicitly say to push `main` without creating a new tag.

3. Review:
   - Default: continue with the repo Operator/Validator review gate before mutating Git.
   - Alternative: say `skip Validator review` to bypass the cross-review for this request.

## Current Evidence

- Current branch: `main`.
- Upstream: `origin/main`.
- Local HEAD: `27e148ae` (`Codify durable combat VFX pipeline`).
- Upstream HEAD: `cdd3f896` (`Prepare alpha-0.8`).
- Existing tag: `alpha-0.8` points at the upstream alpha-0.8 snapshot.
- `Config/DefaultGame.ini` currently reports `ProjectVersion=alpha-0.8`.
- Broad tracked status showed modified/deleted tracked files across `Config`, `Content/Data`, `Content/World`, `Demo`, `Gameplay`, `Scripts`, `Source`, `T66.uproject`, `Tools`, and `ToonStyle/Reports`.

## Stop Rule

No staging, commit, tag, push, reset, clean, or restore should happen until the user answers this decision gate.
