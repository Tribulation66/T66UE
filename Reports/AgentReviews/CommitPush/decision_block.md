# Commit And Push Decision Block

Working task: commit approved current T66 changes to `main`, push `main`, create and push the required next version tag, and verify `main` matches `origin/main` with tracked changes clean or explicitly classified.

Decision gate saved: 2026-06-03.

## Why This Needs User Decision

The live repo state cannot be safely committed under the root instructions without explicit scope and version decisions:

1. `main` is currently aligned with `origin/main` at `25d9e826340bdfb333486df5c927e42867a3583e`.
2. `Config/DefaultGame.ini` currently reports `ProjectVersion=alpha-0.9`.
3. The existing version tags are `alpha-0.1`, `alpha-0.2`, `alpha-0.4`, `alpha-0.5`, `alpha-0.7`, `alpha-0.8`, and `alpha-0.9`; the current project version tag already exists.
4. A commit/push request in root `AGENTS.md` includes creating and pushing the next version tag from the committed `main` tip. The next version value is not named by the user.
5. The tracked tree is not limited to the approved LFS/media/model cleanup. Current tracked status has 203 staged entries and roughly 625 unstaged entries.
6. The unstaged tracked set includes a non-trivial Mini/minigame block. Root `AGENTS.md` excludes Mini/minigame scope unless the user explicitly includes it.
7. The unstaged tracked set also includes broad non-LFS config, runtime source, UI, data, scripts, tools, runtime dependencies, and project-file changes. These appear outside the approved LFS cleanup scope and cannot be silently committed or reverted.

## Decisions Needed

Please answer these before commit/tag/push:

1. Version/tag:
   - Choose the next version/tag value, such as `alpha-0.10` or `alpha-1.0`. Root `AGENTS.md` says this value becomes both `ProjectVersion` and the Git tag.
   - Alternative: explicitly override repo policy and push without a new version tag.

2. Scope for the current dirty tree:
   - Recommended: commit only the approved staged LFS/media/model cleanup plus the chosen version bump, and explicitly defer the unrelated unstaged tracked changes for a later cleanup/commit.
   - Alternative: include all tracked changes, including Mini/minigame paths. This explicitly expands scope beyond the default Mini exclusion and should only be chosen if all 625 unstaged tracked entries are intended for this release.
   - Alternative: pause commit/push until the unrelated tracked changes are separately resolved.

## Current Evidence

- Current branch: `main`.
- Upstream: `origin/main`.
- Local HEAD: `25d9e826340bdfb333486df5c927e42867a3583e`.
- Upstream HEAD: `25d9e826340bdfb333486df5c927e42867a3583e`.
- Current project version: `alpha-0.9`.
- Existing latest tag: `alpha-0.9`.
- Current tracked status summary: 203 staged entries, roughly 625 unstaged entries, with a non-trivial Mini/minigame block.
- Staged scope is the approved LFS/media/model cleanup and its reports.
- Unstaged scope spans `Source`, `Content`, `RuntimeDependencies`, `UI`, `Gameplay`, `Scripts`, `Tools`, `Config`, `AGENTS.md`, and `T66.uproject`.

## Stop Rule

No commit, tag, push, reset, clean, restore, or broad staging should happen until the user answers this decision gate.
