# Optional Validator Protocol

This file is read only when the user explicitly asks for cross-model validation or direct Claude work in `C:\UE\T66`. It is not part of normal startup.

## Default

Normal T66 work is single-agent:

- in Codex, Codex handles the request normally;
- in Claude Code, Claude handles the request normally;
- no other model is invoked unless the user asks for it.

Use validation as a troubleshooting tool: to get unstuck, find a solution to a problem one model has not solved alone, catch missed constraints, or add independent scrutiny to high-risk work.

## User Commands

Per-request examples:

- `validate this with Claude`
- `ask Claude to review this`
- `implement validator Claude`
- `validate this with Codex`
- `ask Codex to review this`

These commands apply only to the current question, stuck step, failed approach, or specific problem being debugged. They do not create a persistent mode.

## Validator Behavior

When validation is engaged:

1. The active model does the normal work or drafts the answer.
2. The validator gives an independent answer or targeted cross-review.
3. The active model incorporates valid corrections and rejects unsupported ones.
4. If only the user can decide, ask once and stop until answered.

The validator is advisory. It is not a second implementation owner by default and should not mutate files, run mutating commands, or drive editor/tooling surfaces unless the user explicitly requests that scope.

Use one correction pass by default. More passes are allowed only when a real blocker remains; do not loop for polish or process satisfaction.

## Claude Validation From Codex

Before running Claude, verify that `ANTHROPIC_API_KEY` is not set in Process, User, or Machine scope. If it is set, stop and ask the user how to proceed.

Use the local Claude Code CLI authenticated to the user's Claude subscription. When Codex needs Claude as validator, use `Scripts\Invoke-ClaudePlanReview.ps1`:

- `-Mode IndependentAnswer` asks Claude for the answer it would give.
- `-Mode CrossReview` asks Claude to compare a Codex draft against the prompt and optional independent answer.

Both modes are advisory. Codex remains responsible for the final user-facing answer in the active Codex workspace.

## Direct Claude Work

Sometimes the user may explicitly want Claude to inspect files directly, perform implementation work, or use editor/tooling surfaces. That is a separate tool-access decision, not the validator path.

Use `Scripts\Invoke-ClaudeDirectRead.ps1` for direct Claude work. For mutating Claude work launched from Codex, Codex must approve the scope first. The helper's `FullOperator` name is a tool profile name only, not a standing T66 role.

Claude-produced artifacts and changes are not automatic greenlights. The active model still checks the actual workspace, outputs, and proof before reporting completion.

## Proof

Do not claim current compile, run, capture, test, editor, or gameplay proof unless that proof was actually attempted in the current task. Prior evidence can inform planning, but it cannot replace a current verification request from the user.

If proof cannot be run, say so directly.

## Helper Output

Validator helper results stay binary:

```text
Result: OK
Result: NEEDS_USER
```

`OK` means the models can resolve the prompt internally. Claude may still list corrections or evidence gaps, but the active model handles them before answering.

`NEEDS_USER` means the user's attention is required because only the user can decide, approve, unblock a missing prerequisite, resolve an unavailable required tool, or change the scope.

## Failure Modes To Avoid

- Do not invoke a validator for every prompt by habit.
- Do not use validator packet ceremony for ordinary answers.
- Do not let the validator mutate files or drive tools without explicit scope.
- Do not ignore valid validator corrections when validation is engaged.
- Do not treat Claude artifacts as final acceptance without checking the actual workspace and evidence.
- Do not continue through a user-only decision.
- Do not claim proof that was not run.
