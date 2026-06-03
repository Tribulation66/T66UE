# Original Prompt

Alpha 1.0 and commit and push everything except what is naturall git ignored

# Task Contract

Working task:
Operator: Codex
Validator: Claude
Scope: update `ProjectVersion` to `alpha-1.0`, include all non-ignored tracked and untracked repo changes in the release commit, push `main`, create and push tag `alpha-1.0`, and verify alignment plus clean tracked state.
Stop condition: commit/tag/push verified, or a hard blocker from Git/LFS/release policy.

# Applicable Repo Rules

- Root `AGENTS.md` is the process router.
- Current `.t66/operator-state.json` selects Codex as Operator and Claude as Validator.
- The user's current reply provides the version/tag decision: `alpha-1.0`.
- The user's current reply expands the dirty-tree scope to everything Git does not naturally ignore, including previously user-gated Mini/minigame paths and unignored untracked files.
- `main` is the only normal development branch.
- When the user names a new version, update `ProjectVersion` in `Config/DefaultGame.ini` and use that exact value for the Git tag.
- A `commit and push` request means commit approved changes to `main`, push `main`, create and push the next version tag, and verify the working tree is clean afterward.
- Never use blanket discard, reset, or clean commands to make the tree clean unless explicitly approved.
- Avoid force-adding ignored files; use normal Git add behavior to respect `.gitignore`.
