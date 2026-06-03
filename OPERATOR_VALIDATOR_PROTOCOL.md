# Operator/Validator Protocol

This file defines the lightweight Operator/Validator process for `C:\UE\T66`.
`AGENTS.md` remains the root router.

## Purpose

The purpose of the stack is simple:

1. The non-Operator receives the original prompt first and produces an independent repo-grounded answer.
2. The Operator independently does the real work and prepares the draft, change, evidence, or answer.
3. The two answers are compared for missed constraints, mistakes, weak evidence, and unclear wording.
4. The final router synthesizes the final answer from the Operator work, the independent Validator answer, and any valid corrections.

Do not turn this into a compliance ritual. Do not use hard review-depth
categories for ordinary work. The point is a quick, targeted, deeper check that
improves answers and implementation without adding unnecessary ceremony.

## Roles

### Operator

The Operator owns the work:

- understand the user's task and current repo state;
- read the relevant live files and folder instructions;
- decide whether the task is clear enough or needs a user question;
- make scoped changes when changes are requested and allowed;
- collect current proof when proof is required;
- prepare the answer, implementation summary, or evidence summary;
- synthesize the final answer after checking the Validator's answer and corrections.

### Validator

The Validator owns independent scrutiny:

- receive the original prompt before the Operator finishes its draft;
- inspect the repo read-only when repo context is needed;
- produce the answer it would have given from the current evidence;
- compare that independent answer with the Operator draft;
- look for mistakes, missed constraints, stale assumptions, weak evidence, and unclear wording;
- patch answer text when that is the cleanest way to improve it;
- return concrete findings when the Operator needs to fix something;
- ask for a user decision only when the user is the only person who can choose the next path.

The Validator is not a second implementation owner by default. It should not
mutate files, run mutating commands, or drive editor/tooling surfaces unless the
user switched roles or Codex approved that scope under the Claude Operator rules.

### Final Router

The final router sends the answer to the user. In the active Codex workspace,
Codex normally remains the final router even when Claude is Operator.

The final router:

- checks the Validator's independent answer and corrections;
- incorporates valid corrections;
- ignores invalid corrections or unsupported claims;
- treats fixable cross-review findings as internal work, not user-facing blockers;
- checks the actual workspace/proof when files, commands, editor work, or captures are involved;
- reports what changed and what verification was performed.

## Current Role State

At task start, read `.t66\operator-state.json` when it exists. That file is the
project-global Operator/Validator state for this machine.

Direct user commands `Make Claude operator` and `Make Codex operator` are
project-global role-switch commands and must be applied through
`Scripts\Set-T66Operator.ps1`.

If the state file is missing, malformed, names an unknown model, or names the
same model for both roles, fall back to request-local routing and tell the user
what state problem was found.

## Normal Loop

When Codex is Operator:

1. Codex receives the prompt, derives the task contract, reads the role state,
   and forwards the original prompt plus task contract and relevant repo rules to
   Claude for an independent read-only answer.
2. Codex independently does the work and prepares the answer, plan, diff summary,
   or evidence summary it would send to the user.
3. Codex sends the Operator draft to Claude for targeted cross-review, preferably
   with Claude's independent answer included as comparison context.
4. Claude compares the two answers and returns concrete corrections, answer
   patches, missing-evidence notes, or user-only decision points.
5. Codex checks Claude's corrections, accepts the valid ones, rejects unsupported
   ones, and sends the synthesized final answer.

When Claude is Operator:

1. Codex receives the user prompt, derives the task contract, reads the role
   state, and immediately forwards the request to Claude with the task contract,
   current role state, relevant repo rules, and scope boundaries.
2. Claude does the Operator work and returns the result.
3. Codex independently checks for mistakes, scope violations, missing evidence,
   or repo-rule conflicts.
4. If Codex finds material issues, Codex sends only those issues back to Claude.
5. Claude revises.
6. Codex does a final sanity check and routes the answer to the user without
   redoing Claude's work.

One correction pass is the normal expectation. More passes are allowed when a
real blocker remains, but do not loop for polish or process satisfaction.

## Requirement

Every prompt goes through this Operator/Validator process when the configured
Validator or Operator tool is available. The default process is independent
answer first, then targeted cross-review.

If the configured Validator tool is unavailable, that needs the user's attention
only when the missing Validator blocks the requested work. Otherwise, report that
it was unavailable and proceed only with the Operator work needed to answer the
user. Do not treat the task as fully cross-reviewed when the independent pass did
not run.

## Boundaries That Still Matter

The simplified process does not remove common-sense safety boundaries:

- If only the user can decide the next path, ask once and stop until the user answers.
- Do not make destructive, hard-to-reverse, credential, billing, Git/LFS,
  release, broad cleanup, migration, production-asset, Unreal/editor automation,
  or similarly high-impact changes without clear scope.
- Do not claim current compile, run, capture, test, editor, or gameplay proof
  unless that proof was actually attempted in the current task.
- Prior evidence can inform planning, but it cannot replace a current
  verification request from the user.
- If Claude will edit files, run mutating commands, or drive editor/tooling
  surfaces, Codex must approve the scope first.
- Operator artifacts and Operator-made changes are not automatic greenlights.
  The final router checks the real workspace, outputs, and proof before
  reporting completion.

These are boundaries, not review categories.

## Claude Billing Guard

Claude runs must use the local Claude Code CLI authenticated to the user's Claude
subscription, not Anthropic API billing.

Before running Claude as Operator or Validator, verify that `ANTHROPIC_API_KEY`
is not set in Process, User, or Machine scope. If it is set, stop and ask the
user how to proceed.

`Scripts\Invoke-ClaudePlanReview.ps1` and the Claude Operator helpers enforce
this by default. Only use `-AllowApiKeyBilling` when the user explicitly accepts
that billing route.

## Claude As Validator For Codex

Use `Scripts\Invoke-ClaudePlanReview.ps1` for Claude's two advisory passes on
Codex-operated work:

- `-Mode IndependentAnswer` sends the original prompt and task contract to
  Claude before Codex finishes its own draft. Claude may inspect the repo
  read-only and returns the answer it would have given.
- `-Mode CrossReview` sends Codex's draft, the original prompt, and, when
  available, Claude's independent answer. Claude compares the answers and
  returns concrete corrections or answer patches.

Both modes are advisory. Codex remains the Operator and final synthesis owner
unless the user changes the role state.

Do not run these helper passes with a tiny max-turn cap such as `-MaxTurns 2`.
Use the helper default unless there is a concrete reason to override it.

## Claude As Operator

Use `Scripts\Invoke-ClaudeDirectRead.ps1` for Claude Operator work.

For read-only Operator work, use the read-only profile or
`Scripts\Invoke-ClaudeReadOnlyOperator.ps1`.

For Claude implementation work that may edit files, run mutating shell commands,
or drive editor/MCP tools, Codex must first write an approval artifact whose
first non-empty line is:

```text
Codex Approval: APPROVE
```

That approval must name:

- approved task;
- approved scope;
- approved tool surface;
- required process rules;
- explicitly excluded actions;
- verification required after the Operator run;
- approval rationale.

Reject or narrow the request if it is vague, asks for destructive work without
clear user approval, hides a user-only decision, violates `AGENTS.md`, or skips a
required proof/process route.

## Proof-Bearing Work

Some tasks need produced proof rather than reasoning alone. Examples include:

- build or compile logs;
- commandlet markers;
- runtime/editor screenshots, dumps, or captures;
- gameplay proof;
- visual judgment of a produced artifact;
- more than one independent proof class in the same task.

If the user asks for current proof, the Operator must attempt that proof unless
it is physically impossible. If proof cannot be run, say so directly.

A read-only Claude run can plan proof-bearing work, but it cannot stand in for
the proof. The implementation/proof phase must run through an approved tool path
that can actually produce the evidence.

For broad proof-heavy work, split the work into practical phases when that keeps
scope and verification understandable. Do not phase work just because a template
says so.

## Validator Helper Output

`Scripts\Invoke-ClaudePlanReview.ps1` is the normal helper for Claude's
independent-answer and cross-review passes on Codex-operated work.

The helper asks Claude to include one simple result line. There are only two
outcomes:

```text
Result: OK
Result: NEEDS_USER
```

Meanings:

- `OK`: the models can handle the prompt internally. Claude may still list
  corrections, evidence gaps, or wording patches, but they are fixable by the
  Operator/final router before answering.
- `NEEDS_USER`: the user's attention is required because only the user can decide,
  approve, unblock a missing prerequisite, resolve an unavailable required tool,
  or change the scope.

The parser accepts a clear `Result:` line anywhere in the response and can infer
the same two outcomes from unambiguous wording. It must not reject an otherwise
usable Claude answer just because Claude wrote a short sentence before the
result line.

After the result line, Claude should keep the output short and practical. The
exact headings can vary by mode, but they should support the same goal:
independent answer first, then targeted correction.

This is not a packet-completeness ritual. It is just a reliable shape for useful
cross-model scrutiny.

## Token Notes

Use available helper token counts when they are exposed.

For final user-facing answers, `Scripts\Get-CodexTokenUsage.ps1` is the read-only
source for Codex token data. It reports the latest completed Codex turn before
the final answer; the final answer's own tokens are not included until after the
answer is sent.

For Claude helper runs, use `ClaudeTokensSpent` from the helper output, manifest,
or `claude_tokens.json` sidecar when present. Plan-review helpers should use
Claude JSON output by default so token usage can be captured. If the helper does
not expose a count, report `Unavailable`. Report `0` for Claude only when Claude
was genuinely not invoked.

Do not delay a useful answer only to chase unavailable token data.

## Report Artifacts

When a durable report, handoff packet, review artifact, or proof summary is
needed, store it under `Reports/` and follow `Reports/AGENTS.md`.

Ordinary answers do not need report artifacts.

## Failure Modes To Avoid

- Do not wait to contact the Validator until after the Operator has fully
  committed to a final answer.
- Do not use packet ceremony for ordinary answers.
- Do not use hard review-depth labels as a substitute for judgment.
- Do not let the Validator mutate files or drive tools without approved scope.
- Do not let the Operator ignore valid Validator corrections.
- Do not let Claude mutate files or drive tools without Codex-approved scope.
- Do not treat a Claude artifact as final acceptance without checking the actual
  workspace and evidence.
- Do not continue through a user-only decision.
- Do not claim proof that was not run.
