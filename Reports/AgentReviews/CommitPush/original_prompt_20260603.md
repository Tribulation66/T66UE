# Original Prompt

commit and push

# Task Contract

Working task:
Operator: Codex
Validator: Claude
Scope: commit approved current repo changes to `main`, push `main`, create and push the next version tag if the live repo policy/version state makes that unambiguous, and verify alignment/clean tracked state without broad destructive cleanup.
Stop condition: pushed commit/tag verified, or a decision gate if the required tag/version choice or dirty-tree scope is ambiguous.

# Applicable Repo Rules

- Root `AGENTS.md` is the process router.
- Current `.t66/operator-state.json` selects Codex as Operator and Claude as Validator.
- Default scope excludes Mini/minigame systems unless explicitly named.
- A `commit and push` request means commit approved changes to `main`, push `main`, create and push the next version tag, and verify the working tree is clean afterward.
- The task is not complete until `main` equals `origin/main` and there are no remaining tracked changes. If tracked changes remain, classify each as commit, restore, ignore/untrack, or explicitly deferred.
- Never use blanket discard, reset, or clean commands to make the tree clean unless the user explicitly approves that destructive cleanup.
