Result: OK

## Independent Answer

This is a well-defined release task that Codex can execute without further user input — the user already supplied the version/tag decision (`alpha-1.0`) and explicitly expanded scope to "everything except what is naturally git-ignored."

Concrete steps for the Operator:

1. **Bump version** — edit `Config/DefaultGame.ini:7`, `ProjectVersion=alpha-0.9` → `ProjectVersion=alpha-1.0`.
2. **Stage all non-ignored changes** — use normal add behavior (`git add -A`), which respects `.gitignore`. Do **not** use `-f`/force-add and do **not** use any discard/reset/clean to "tidy" the tree.
3. **Commit** to `main` (follow the existing convention: `Prepare alpha-1.0`).
4. **Push** `main`.
5. **Tag** `alpha-1.0` (matches the existing `alpha-0.x` series; `alpha-1.0` does not yet exist) and push the tag.
6. **Verify** afterward: working tree clean for tracked files, local `main` aligned with `origin/main`, tag present locally and on remote.

## Evidence Checked

- `Config/DefaultGame.ini:7` → currently `ProjectVersion=alpha-0.9` (needs bump).
- Tag list: `alpha-0.1, 0.2, 0.4, 0.5, 0.7, 0.8, 0.9` exist; **`alpha-1.0` does not exist** — safe to create. (The `vX.Y` tags are a separate legacy scheme; the alpha series is the current one.)
- Branch is `main`; latest commit `25d9e826 Prepare alpha-0.9` confirms the "Prepare alpha-X" commit convention.
- `git status --short` → 829 changed tracked entries (mix of M and D, including staged D's), and **0 untracked (`??`) files**.

## Questions Or Blockers

None requiring the user. The "unignored untracked files" clause in the contract is moot — there are currently zero untracked files, so nothing extra to decide there.

## Caveats

- The dirty tree mixes staged and unstaged states (e.g. `D ` in column 1 vs ` M`/` D` in column 2). `git add -A` will reconcile both; confirm the resulting commit captures every intended deletion and modification.
- Many changes are LFS-tracked binaries (`.uasset`, `.ogg`) plus numerous **deletions** (Arcade audio, Deck/Idle CSVs, ArcadeInteractables). These deletions are part of the approved scope per the user's "everything" instruction, but they are destructive content removals — worth a sanity glance that they're intended before the commit, not after the push.
- I could not confirm `origin/main` ahead/behind status or the remote URL (those read-only commands were blocked/cancelled). Codex should verify remote alignment as part of the post-push check.
- `.gitattributes` and `.gitignore` are themselves modified in this tree; since staging happens after they're committed in the same commit, double-check no newly-ignored path is unintentionally dropped or retained.
