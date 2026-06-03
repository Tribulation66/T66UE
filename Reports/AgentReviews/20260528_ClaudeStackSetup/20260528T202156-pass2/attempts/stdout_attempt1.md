Verdict: REVISE

## Blockers

None hard-blocking, but see Major Issues for items that should be tightened before edits start.

## Major Issues

- **Redundant/conflicting `--tools` and `--allowedTools` in baseline profiles.** Both flags are passed with identical values in the AGENTS.md text, the registry row implication, and the new `Invoke-ClaudeDirectRead.ps1` defaults. The packet claims `claude --help` confirms both flags exist, but does not explain why both are needed simultaneously or what happens if their sets diverge. Pick one as the source of truth (likely `--allowedTools`) and document precedence, or justify the belt-and-suspenders pairing. Otherwise the baseline profile becomes a footgun the moment a future task widens one but not the other.
- **Model ID `claude-opus-4-8` is asserted but not defensively handled.** The packet cites a probe artifact and Anthropic docs, but the helper has no fail-closed branch if `--model claude-opus-4-8` returns "unknown model" on the user's subscription/CLI build. The preflight checks auth, not model availability. Add either a one-shot model probe (e.g., `claude --model claude-opus-4-8 -p "ok"` with bounded timeout) inside the preflight, or document that a model-resolution failure must be surfaced verbatim in the run manifest and abort the run.
- **`claude auth status --json` schema is assumed, not pinned.** The preflight will fail closed unless `loggedIn=true`, `authMethod="claude.ai"`, and `apiProvider="firstParty"` appear. The packet says the subcommand was inspected but does not quote a real JSON sample showing those exact field names and string values. If any field name or casing differs (e.g., `provider` vs `apiProvider`, `subscription` vs `claude.ai`), every run fails closed. Capture a sanitized fixture once, pin the schema in the test, and treat a schema mismatch as a fail-closed condition distinct from "logged out".
- **Operator-mode artifact is correctly labeled non-greenlight, but the routing back to a reviewer is not specified.** The packet says operator output is "not a greenlight" and must be cross-validated, yet does not state which helper or path the validator critique gets written to, or whether the operator manifest must reference the corresponding review artifact path once it exists. Without that link, a future agent could ship operator artifacts without a paired review and still satisfy the letter of the AGENTS rule.

## Minor Issues

- The new AGENTS.md subsection text and the registry row paraphrase overlap heavily. Risk of drift if one is edited later. Consider keeping the subsection canonical and making the registry row a one-line pointer.
- The operator-mode default `--effort high` plus 180s timeout / 2 attempts is reasonable for review, but operator passes that legitimately need deep reads may time out silently. Either raise default operator timeout or require operator callers to pass `-TimeoutSeconds` explicitly.
- Blender MCP setup is bundled into the same reviewed pass as the AGENTS/script changes. It is a config write to user scope and the packet correctly says to abort on conflict, but it is a different blast radius than doc edits. Consider splitting it into a separately verified step so the doc/script change is not held hostage to MCP success.
- The Accepted Process Registry rows are drafted without confirming current registry column count/format. If the existing table uses different column headers, the inserted rows will look wrong.
- The Unreal/Niagara guidance ("desktop screenshots are not acceptance proof") is stated in the AGENTS subsection and again in the registry row and again in Scripts/README. Three statements of the same rule across three files is fine for emphasis but increases drift risk; pick a canonical home.
- No explicit rollback note: if the helper script lands but the auth-fixture tests later reveal the JSON schema is wrong, the rollback path (revert script + AGENTS subsection) should be named so future agents know the unit of revert.

## Clarifying Questions

- Is `claude-opus-4-8` confirmed available on the user's first-party Claude Code subscription right now, not just listed in API docs? The probe artifact reference is from 2026-05-28 (today). If yes, link the probe path in the helper comment so future agents can re-verify.
- Are `--tools` and `--allowedTools` semantically different on this CLI build, and if so, which one actually constrains tool availability when they disagree?
- Should the operator manifest be required to embed (or be embedded by) the validator's review manifest path, so a `find` over `Reports/AgentReviews/ClaudeDirectRead/` can detect unpaired operator artifacts?
- Is there an existing convention for `<timestamp>-<task>` naming in `Reports/AgentReviews/` (e.g., `20260528T200833-pass2/` from the prior approved pass uses a different shape)? If so, the new folder should match.

## Required Verification

Before Codex starts edits, the packet should commit to:

- A model-availability probe inside the preflight, with its outcome included in the run manifest.
- A pinned `claude auth status --json` fixture (sanitized) used by the parser test, plus negative fixtures for: logged-out, API/key auth, unknown provider, malformed JSON, missing field.
- A confirmation that `--tools` and `--allowedTools` together do not produce a CLI parse error or unintended widening.
- Working-tree check after each smoke run, scoped to the touched script/doc paths (already in the plan — good).
- Strict verdict-parser test still green after model/effort parameterization (already in the plan — good).
- For the Blender MCP step: explicit verification that `claude mcp list` shows the new `blender` entry only at user scope and not at project scope, and that no pre-existing `blender` entry was overwritten.

## Rationale

The packet correctly preserves the cross-validation invariant, keeps Claude baselines read-only with `--permission-mode plan`, defers quota/denominator work as the user instructed, scopes Mini out, and explicitly forbids `Edit`/`Write`/`bypassPermissions`/Unreal Python from the baseline. The AGENTS.md subsection and registry row design are well aligned with the user's "new agents must not miss the new process" requirement, and the operator/reviewer profile split is sensible. The scope (one new helper, one helper update, AGENTS + two Reports docs + Scripts README + one MCP config) is large but matches the user's explicit ask.

Revisions needed are operational: confirm model availability defensively rather than by reference, resolve the `--tools` vs `--allowedTools` duplication, pin the auth JSON schema with a real fixture, and specify how operator artifacts get paired with their validator critique so the cross-validation rule cannot be quietly skipped. These are Codex-resolvable without product-direction input, so REVISE rather than NEEDS_HUMAN_DECISION or BLOCK.

