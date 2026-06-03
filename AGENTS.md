# AGENTS.md

## 1. Project Contract

- Treat this file as the root process router for `C:\UE\T66`. It defines global behavior, accepted process classes, and where to find deeper folder-owned instructions.
- Before answering or acting on every new user request or question, derive a plain task contract. Do not call native goal tools for T66 work: do not create, update, complete, block, or query goals (`/goal`, `set_goal`, `create_goal`, `update_goal`, `get_goal`, or equivalent native goal APIs) as part of repo process. If the host app displays, injects, or exposes goal context anyway, treat it as non-authoritative telemetry only and ignore it for task control; it does not control scope, stop conditions, continuations, blocking, completion, or whether work may continue.
- The task contract is the active scope source. Keep it lightweight and prompt-native:

```text
Working task:
Operator:
Validator:
Scope:
Stop condition:
```

Use the task contract to decide what files/systems to inspect, what changes are in scope, and what verification proves the request is done.
- Start from the live repo, current folder instructions, current assets, current scripts, and current machine state. Do not answer from stale docs or memory when the live project can be checked.
- After deriving the task contract and before Operator/Validator routing, read `C:\UE\T66\.t66\operator-state.json` when it exists. Treat that local repo file as the project-global T66 Operator/Validator state for this machine. If it is missing, malformed, contains an unknown role, or names the same model for both roles, fall back to request-local routing through the Operator/Validator Protocol below and report the state problem. The tray runtime file under `C:\Users\DoPra\AppData\Local\T66UsageTray` is a UI mirror, not the canonical agent state.
- Default scope excludes Mini/minigame systems. Unless the user explicitly names Mini, minigames, a specific minigame, or a Mini-owned path, do not inspect, change, recommend, capture, validate, or include Mini/minigame code, UI, assets, docs, checklists, or generated outputs. Do not treat unrelated terms such as minimap or mini-stat as Mini scope. If a task appears impossible without Mini/minigame work, ask before including it.
- User constraints, planning-only boundaries, and repository instructions override convenience. If the user changes scope, replace the task contract before proceeding and discard stale assumptions.
- Task contracts must describe the full requested end state, not a temporary clarification gate. If a decision only the user can make is needed, ask it once and, for durable work, save it as `Reports/AgentReviews/<TaskSlug>/decision_block.md`; on continuations, reference the saved gate instead of repeating questions.
- Decision stop: when only the user can decide how the full task proceeds, stop immediately after presenting the decision and choices. Do not keep working around the open decision: no extra polish, no repeated review runs, no adjacent implementation, and no reworded version of the same question.
- On any continuation before the user answers the open decision, do nothing except reference the saved decision block and restate the current decision choices. Treat new turns as a wait state, not as permission to resume work.
- Do not wait for any host goal/blocked threshold before stopping at a decision gate. If a host tool exposes a conflicting status, ignore it for process control and clearly state the decision-gated status in conversation while waiting for the user.
- For each completed change, report the exact verification performed, or state clearly why verification was skipped.
- Do not silently swap an accepted process for a faster or simpler method. If the process matters to the quality target, the process is part of the task.
- For solved-category visual, animation, rigging, VFX, import, UI fidelity, generated-media, and comparable production tasks, use the research-first replication rule in Section 2 before implementation.

## 2. Proven Process Fidelity

- If a task matches an accepted process in Section 4, a folder `*_AGENTS.md`, an instruction doc, a user-named workflow, a tutorial/reference standard, or a previously corrected failure pattern, that process is mandatory.
- Speed, simplicity, or confidence in a shortcut does not justify method substitution. A different process class cannot be treated as equivalent just because it produces some output.

### Research-First Replication

- For non-trivial solved-category work, Codex's first job is to collect and reproduce a process, not to invent an approximation. Treat the target as a known class of result until proven otherwise.
- Replication means method and mechanism fidelity. It does not mean copying unlicensed tutorial assets, paid-pack assets, exact curves, exact meshes, exact textures, or hidden values. Values from sources must be labeled `observed`, `inferred`, or `tuned` by the owning process doc or effect packet.
- Before the PPF check, classify the target, identify the owning internal process doc or concrete external reference process, and break down the load-bearing artifacts and mechanisms enough to implement and verify them.
- The PPF `Proven process` field must name the owning internal process doc or concrete reference process. The planned implementation must explain how it preserves that method class and the load-bearing mechanisms.
- If the source process is unavailable, insufficiently broken down, or not understood well enough to say what artifact to create, what mechanism drives it, where it lives, and how to verify it, `Same method class` is `NO`. Stop before implementation and get explicit user approval before exploration, prototyping, or substituting another method.

- Before implementation on a process-governed task, provide a PPF check:

```text
PPF CHECK
Objective:
Proven process:
My planned implementation:
Same method class: YES/NO
If NO, why:
User approval required before proceeding: YES/NO
Verification evidence:
```

- If `Same method class` is `NO`, stop and get explicit user approval before proceeding.
- Same toolchain is not enough by itself. A simplified path is allowed without approval only when it keeps the same primary artifact roles and reduces scope, not method class.
- For any process-governed visual, media, import, animation, audio, or VFX task, include an artifact parity gate before implementation:

```text
ARTIFACT PARITY GATE
Reference artifact/category:
Role: Primary/Secondary
Required: YES/NO
Planned artifact/path:
Status: SAME/EQUIVALENT/MISSING/DEFERRED
Evidence:
```

- Dropping, deferring, or replacing a required primary artifact means `Same method class` is `NO` unless the user explicitly approves the substitution.
- Generic seed/support effects cannot satisfy a required primary visual carrier. For Niagara VFX, the slash, trail, projectile, aura, or attack silhouette must be produced inside the Niagara asset, its renderer material, its renderer mesh/ribbon, or its emitter logic; actor-side component transforms may place the system in the world but may not be the source of the silhouette.
- A required primary artifact marked `EQUIVALENT` or `DEFERRED` requires explicit user approval before implementation.
- Example: simpler Niagara is acceptable for a Niagara VFX task when the primary silhouette is still authored by Niagara/material/renderer assets; replacing that silhouette with procedural C++ debug geometry or actor-arranged point components is not.
- Satisfying a process label, method class, or artifact category is necessary but not sufficient. A process-governed result is complete only when the required mechanisms that make the reference work are individually present and evidenced.
- For process-governed visual, media, animation, audio, generated-content, and VFX tasks, add a mechanism manifest after the artifact parity gate and before implementation. Mechanisms are verbs or behaviors, not nouns: motion, sampling, timing, panning, erosion, dissipation, blending, masking, import/reload, capture, validation. A mechanism is required when removing it changes the identity of the result rather than only reducing polish.
- Required mechanisms must come from the owning process doc, effect packet, or reference/source breakdown. Do not infer missing load-bearing mechanisms from vague target wording. If a mechanism is not understood well enough to implement and verify, mark it `MISSING` and stop before implementation.

```text
MECHANISM MANIFEST
Reference/source:
Required mechanisms:
  1. Mechanism:
     Required: YES/NO
     Planned implementation:
     Evidence needed:
  2. Mechanism:
     Required: YES/NO
     Planned implementation:
     Evidence needed:
```

- Same toolchain plus matching artifacts cannot pass if required mechanisms are missing. If any required mechanism is `MISSING`, `DEFERRED`, or replaced by a different behavior class, report `Same method class: NO` or `PARTIAL` and get explicit user approval before proceeding or accepting the result.
- Anti-lookalike rule: name the cheapest wrong result that could pass a weak gate, then name and prove the discriminator that separates the intended result from that lookalike. A static crescent mask is not a slash; a slash needs a moving, time-layered carrier with reveal and dissipation. A still image is not video proof; temporal behavior needs frame-range evidence.
- At completion on a process-governed task, provide a PPF close:

```text
PPF CLOSE
Process used:
Matches declared process: YES/NO
Evidence:
```

- For any task that required a mechanism manifest, also provide a mechanism close:

```text
MECHANISM CLOSE
Mechanism:
Status: PRESENT/ABSENT/DEFERRED
Evidence:
Discriminator test:
Reported status: FULL/PARTIAL
```

- `FULL` requires every required mechanism to be `PRESENT` with evidence. Any `ABSENT` or `DEFERRED` required mechanism means `PARTIAL`; do not describe a partial result as complete. Temporal mechanisms such as motion, animation, timing, reveal, erosion, and dissipation require multi-frame evidence, not a single still.

- Skip PPF only for trivial tasks where QA would care only that the value changed, not how it was produced, such as a single config value, typo, rename, or isolated data-row edit. Do not use this exemption for materials, meshes, particles, UI layout, animation, audio, staged builds, imports, or generated media.

## 3. Planning And Review Loop

### Task Clarification

- After deriving the task contract, decide whether the request is clear enough to plan safely.
- If ambiguity would change scope, ownership, acceptance criteria, risk, verification, or whether files should be edited, ask concise clarifying questions and wait for the user's answer.
- Every clarification step must make the user's clarification opt-out explicit: the user may say `no clarification needed` to proceed on stated assumptions. This does not bypass the Operator/Validator cross-review.
- If no clarification is needed, state the safe assumptions and continue through the Operator/Validator loop.

### Folder Instruction Discovery

- Before acting, infer which project folder owns the request. The user may describe the task by desired end state rather than by folder name.
- Use task wording, repo search, paths, READMEs, and existing docs to identify the responsible folder.
- Read that folder's `*_AGENTS.md` before editing files or running workflow commands.
- If the task crosses folders, read each relevant folder agent file and follow the most specific applicable instructions.
- Folder agent files are routers. They point to required instruction files; they do not replace those files.
- If no folder agent exists, read the nearest `README.md` and relevant instruction docs, proceed conservatively, and report the missing router as a documentation gap.

### Implementation Planning

- Before implementation, create an implementation plan for every request that may change code, content, scripts, docs, configuration, build outputs, releases, or workflow rules.
- For question-only requests, create a short answer plan when repo inspection, external review, or non-trivial reasoning is needed.
- The plan must include the decision-making details needed for the task: clarified task, user constraints, files/folders/systems to inspect or edit, applicable `*_AGENTS.md` and instruction files, intended edit scope, out-of-scope items, risks, rollback or revert considerations, and verification evidence required before completion.
- Keep the plan proportional. Do not create packet ceremony for ordinary answers, narrow fixes, or simple repo questions.

### Simplified Operator/Validator Loop

- Every prompt begins by sending the original user request, task contract, and relevant repo rules to the configured Validator for an independent repo-grounded answer before the Operator finishes its own draft, when the Validator tool is available.
- The Operator still owns the real work: discovery, planning, edits, command/editor work, evidence collection, and final synthesis.
- The Validator's first pass is an independent answer, not a ceremony packet and not a second implementation owner. The Validator may inspect the repo read-only, surface missed context, and draft the answer it would have given.
- After the Operator has its draft, the Operator and Validator answers are compared. The Validator looks specifically for mistakes, missed constraints, risky assumptions, missing evidence, and unclear wording in the Operator draft; the Operator also checks the Validator answer for invalid corrections or unsupported claims.
- The final router synthesizes from both answers and the valid corrections. Valid corrections are incorporated; invalid corrections are ignored; unresolved user-only decisions are asked once and stop the task.
- Validator helper results stay binary: `OK` when the models can resolve the prompt internally, or `NEEDS_USER` when the user's attention is required for a decision, approval, hard blocker, or unavailable required tool. The parser may infer that result from clear wording and must not fail just because Claude wrote a short sentence before the result line. Ordinary fixable review findings do not become a blocking result; the Operator handles them before answering.
- Do not classify normal work with hard review-depth labels. Use plain judgment. The default process is independent answer plus targeted cross-review, kept as small or as careful as the task needs.
- Every prompt goes through the Operator/Validator process. If the configured Validator tool is unavailable, report the unavailable Validator explicitly and proceed only with the Operator work needed to answer the user.
- Keep these safety stops even under the simplified loop: ask the user when only the user can decide; do not make destructive, credential, billing, Git/LFS, release, broad cleanup, migration, production-asset, Unreal/editor automation, or hard-to-reverse changes without clear scope; do not claim current proof that was not actually run; do not let Claude make mutating changes without Codex-approved scope.
- Report model token spend when available, especially when Claude was invoked. Do not block a useful answer just to chase a missing token count.

```text
**Codex Token Spent:**
<number with comma thousands separators, 0 if unused, or Unavailable if no count is exposed>

**Claude Tokens Spent:**
<number with comma thousands separators, 0 only if Claude was not invoked, or Unavailable if the helper did not expose a count>
```

- Before final user-facing answers, run `Scripts\Get-CodexTokenUsage.ps1` (read-only, no goal tool). When it returns `Available = $true`, populate `Codex Token Spent` from `CodexTokenSpent`. That figure is the latest completed Codex turn before the final answer — the final answer's own tokens are not included until that turn flushes its own `token_count` event after the answer is sent, so label/qualify it accordingly.
- If the helper is unavailable or returns `Available = $false` (no rollout token data), report `Codex Token Spent` as `Unavailable`.

Use available host/helper token counts; when a count is not exposed, report `Unavailable`. Claude plan-review helpers should expose `ClaudeTokensSpent` from JSON output or `claude_tokens.json`; use that when present. Report `0` for Claude only when Claude was genuinely not invoked; a failed or incomplete Claude run still reports its parseable usage, otherwise `Unavailable`.

### Operator/Validator Protocol

- For requests that use the Claude/Codex Operator and Validator stack, follow `OPERATOR_VALIDATOR_PROTOCOL.md`. That file owns the simplified loop, role routing, Claude tool profiles, approval boundaries, proof ownership, and token notes.
- Core invariant: the non-Operator gets the original prompt first and produces an independent repo-grounded answer; the Operator produces its own work or draft; both outputs are cross-checked for oversights; the Operator/final router synthesizes the final result.
- The Validator is the non-Operator model unless the user explicitly names a different reviewer. Direct user commands `Make Claude operator` and `Make Codex operator` are project-global operator-switch commands and must be applied with `Scripts\Set-T66Operator.ps1`.
- Before running Claude as Operator or Validator, verify that `ANTHROPIC_API_KEY` is not set in Process, User, or Machine scope. If it is set, stop and ask the user how to proceed rather than risking API charges.
- Claude Operator and Validator runs must use the local Claude Code CLI authenticated to the user's Claude subscription. Use `Scripts\Invoke-ClaudeDirectRead.ps1` for Claude Operator work and `Scripts\Invoke-ClaudePlanReview.ps1` for Claude independent-answer and cross-review passes on Codex-operated work. Do not use Claude `plan` permission mode for Operator runs. For an approval-free read-only Operator run, use `Scripts\Invoke-ClaudeReadOnlyOperator.ps1`; pass `-Preflight` to any of these helpers to print the effective configuration (timeout policy, effective tool surface, mutating capability, approval status) without invoking Claude.
- Verification freshness is a hard rule: when the user explicitly asks for current compile, run, capture, test, or editor verification, the Operator must attempt that exact current verification unless it is physically impossible. Recent or prior evidence cannot satisfy an explicit current-verification request; if it cannot be run, say so explicitly instead of substituting older evidence. See `OPERATOR_VALIDATOR_PROTOCOL.md`.
- Broad implementation tasks may be split into phases when that is the clearest way to keep scope, proof, and rollback understandable. Do not phase work just to satisfy ceremony.
- When Claude is Operator for substantive implementation, Codex is primarily a wrapper/router, approval gate, and final checker. Codex should forward the user's request plus the relevant task contract and repo rules to Claude instead of deeply planning the work itself. Codex must approve or reject the requested change scope before Claude makes changes. Approved Claude Operator runs may use the full Claude Code tool surface through `Scripts\Invoke-ClaudeDirectRead.ps1 -ToolProfile FullOperator`, including edits, shell commands, and configured MCP/editor tools, inside the Codex-approved scope.
- Proof-bearing work (tasks whose acceptance needs produced build logs, commandlet markers, runtime/editor captures, gameplay proof, visual judgment, or multiple proof classes) routes to FullOperator: a read-only Operator pass may plan it but the implementation/proof phase must be a Codex-approved FullOperator run. Claude-produced proof is evidence, not final acceptance; Codex validates the actual evidence and stays the final reporter. `OPERATOR_VALIDATOR_PROTOCOL.md` owns the proof-routing and final-proof-owner rules.
- Operator artifacts and Operator-made changes are not automatic greenlights. In the active Codex workspace, Codex remains responsible for the task contract, Codex approval, validation of actual changes, excluded operations, final verification review, and the user-facing completion report unless the user explicitly changes that.
- Quota denominator inference and automatic usage routing are deferred until a separate usage-ledger/router pass is reviewed and implemented.

### Delegation And Error Handling

- When the current task can be completed faster by splitting independent work, use available sub-agent or delegation tools instead of doing every task serially.
- Delegate only concrete, bounded work that can run in parallel without blocking the critical path. For code edits, keep write scopes disjoint and tell agents not to revert or overwrite user changes or peer work.
- The lead agent remains responsible for integrating results, resolving conflicts, and reporting verification evidence.
- If the same command, build, test, or runtime path fails twice with the same error signature, stop repeating similar fixes. Check repo-local docs/logs first, then use web research when the cause is not clear.
- Identify 3-5 plausible fixes, choose the smallest repo-appropriate solution, implement it, and report the options considered plus verification evidence.

### Pending Issues

- When making changes to the codebase, any out-of-scope problem encountered must be documented in a `pending_issues_<foldername>.md` file located in the same folder as the affected code or data.
- Format each issue with a heading, severity tag `[Blocker]` / `[Major]` / `[Minor]`, what's wrong, why it is out of scope now, and what fixing it would entail.
- Before working in any folder, read its `pending_issues_*.md` file if present so you do not duplicate or contradict prior decisions.

## 4. Accepted Process Registry

| Process | Trigger | Required process and prohibited substitutions | Verification / reference |
|---|---|---|---|
| Validator review | Every T66 prompt when the configured Validator tool is available, plus any task where the user explicitly asks for review | Follow `OPERATOR_VALIDATOR_PROTOCOL.md`. The non-Operator model is the Validator unless the user explicitly names a different reviewer. The Validator receives the original prompt first, creates an independent repo-grounded answer, then cross-reviews the Operator draft against that answer. When Claude validates Codex, use local Claude Code CLI through `Scripts\Invoke-ClaudePlanReview.ps1`. Verify no `ANTHROPIC_API_KEY` in Process/User/Machine, and do not use Anthropic API billing. Do not use hard review-depth categories or packet ceremony for ordinary answers. | Report Operator, Validator, output scope, caveats, verification performed or skipped, and token data when available. |
| Operator/Validator stack | User says `Make Claude operator` or `Make Codex operator`, asks to shift heavy work between Claude/Codex, conserve one model's usage, or use Claude as primary worker | Follow `OPERATOR_VALIDATOR_PROTOCOL.md` as the detailed authority. Read `.t66\operator-state.json` at task start when present, and update it through `Scripts\Set-T66Operator.ps1` for operator-switch commands. Use Claude Code `claude-opus-4-8` as the default heavy Operator when the state or user request selects Claude. When Claude is Operator, Codex should act as wrapper/router first: immediately package the user's task and repo rules for Claude, avoid deep duplicate planning before Claude's Operator pass, approve or reject Claude's requested change scope before mutating work, then check the actual result. When Codex is Operator, Claude's independent answer and cross-review are advisory inputs to Codex's final synthesis. Quota denominator inference remains deferred until a separate usage-ledger/router pass is reviewed and implemented. | Report Operator, Validator, model/helper used when relevant, Codex approval artifact path for mutating Claude runs, Operator artifact paths, verification, and token data when available. Operator artifacts and Operator-made changes are not greenlights unless Codex checks them under the protocol. |
| Claude tool access | Claude needs direct repo/file, shell, Blender, Unreal, Niagara, or editor visibility as Operator or Validator | Use `Scripts\Invoke-ClaudeDirectRead.ps1`. For Claude Operator implementation, use `-ToolProfile FullOperator` with a valid Codex approval artifact and non-plan permission mode. Full Operator mode may use edits, shell commands, and configured MCP/editor tools inside the approved scope. Use user-scoped Blender MCP only after smoke verification. Use Unreal-owned capture/dump scripts for Unreal/Niagara evidence; do not accept desktop screenshots or raw GUI observation as proof. | Claude helper artifact path, manifest `ClaudeTokensSpent` when exposed, Codex approval artifact path for mutating runs, actual-change validation, Blender MCP list/get output when used, and Unreal-owned artifact paths for visual/editor evidence. |
| Image generation | New bitmap visuals, mockups, sprites, reference images, or image edits | Use the approved built-in `imagegen` / account-backed image generation path. For repo-bound mockups that would clutter the main chat, use a separate local Codex CLI worker with the same account-backed imagegen process for prompt, artifact, variant, and generation management. Do not require, revive, or fall back to `OPENAI_API_KEY` API scripts when account-backed imagegen is the intended process. | Generated image artifact plus visual review; for UI prompts use the self-contained prompt contract under `UI/Reference`. |
| Unreal video and screenshots | Gameplay/UI visual proof, VFX filming, capture requests | Use Unreal-owned capture paths, not desktop screenshots. For VFX/gameplay video use `Scripts/CaptureT66GameplayVideo.ps1`; for UI use `CaptureT66UIScreen.ps1` or `CaptureT66UIWidget.ps1`. Before judging visual proof, perform a diagnostic visibility check: target size, angle, occlusion, temporal sampling, and whether the frames prove the stated gate. Diagnostic captures/contact sheets may help understand an artifact, but they do not replace a locked acceptance camera or gate unless the user explicitly approves that new gate. | Output PNG/MP4 path, frame/log evidence, visibility check result, and ffmpeg/ffprobe check when relevant. |
| Loot UI animation | Loot crate, loot chest, loot bag, loot wheel, or other post-interaction 2D/UI reward animations | Use Section 2 research-first replication plus `UI/Processes/LootUIAnimationAuthoringProcedure.md`. Video references require user-provided transcripts; non-video written references should be read directly. The primary animation carrier must be a target-owned UI surface, not a mislabeled shared card or static result. Do not include Mini/minigame scope unless explicitly named. | Per-target mechanism packet, artifact parity gate, focused build, Unreal-owned capture, ffprobe/frame evidence, and anti-lookalike discriminator proof. |
| Niagara combat VFX | New combat VFX, slash, aura, projectile trail, elemental attack, or weapon effect | Use Section 2 research-first replication plus Niagara/material/texture workflow: texture masks, material animation, emitters/ribbons/sprites/meshes, timing curves, and reference/source analysis. When an effect packet defines a mockup visual target, follow the additive visual-target gate in `Gameplay/Combat/CombatVFXAuthoringProcedure.md`; the mockup controls visual direction only and cannot replace source-method mechanisms. The primary silhouette must live in the Niagara system, renderer material, renderer mesh/ribbon, or emitter logic. Do not replace with procedural C++ debug geometry, static mesh actor layers, actor-arranged point components, or static lookalike masks unless explicitly approved. Slash/trail/aura tasks require a mechanism manifest for motion, timing, material animation, and dissipation before implementation. | Inspectable Niagara/material assets plus Unreal-owned capture, visual readability review, and multi-frame evidence for temporal mechanisms. Reference `Gameplay/Combat/CombatVFXAuthoringProcedure.md` for generic authoring gates and `Gameplay/Combat/Hero1AxeVFXPlan.md` for Hero 1 axe lab work. |
| UI reference fidelity | UI screen implementation or edit from a reference image | Follow `C:\UE\T66\UI\Instructions\UI_FIDELITY_LOOP_INSTRUCTIONS.md`. Do not accept "looks roughly right" or compile success without screenshot/dump/fidelity evidence. | `VerifyUIFidelity` report with zero FAIL items, or documented accepted content deltas. |
| Pixal3D ToonStyle production import | Pixal3D asset entering playable content | Use `Model Generation/Instructions/09_PIXAL3D_TOONSTYLE_PRODUCTION_IMPORT_INSTRUCTIONS.md`. Do not use legacy one-off import scripts or manual ToonStyle material assignment. | Manifest, Blender pipeline output, Unreal import, hard validators, and staged build proof if playable content is affected. |
| Staged standalone verification | Any change affecting the playable standalone build or taskbar shortcut | Refresh staged standalone and verify shortcut target. Do not treat source compile as proof of what the taskbar launches. | `Scripts\StageStandaloneBuild.ps1`, shortcut target `C:\UE\T66\Saved\StagedBuilds\Windows\T66\Binaries\Win64\T66.exe`, smoke logs. |
| Video generation | Frontend/hero-selection generated videos or existing video-generation workflow research | Start from `Video Generation` docs, manifests, job records, and validators. Do not jump straight to ad hoc ffmpeg or generation scripts. | Manifest/job counts, runtime movie paths, validator output, and encoded media evidence. |
| Data asset import/reload | Source CSV/JSON/PNG changes that back Unreal DataTables or cooked assets | Update source data, then run the owning Unreal commandlet/import script. Do not stop at file edits when runtime assets must be refreshed. | Script log markers such as DataTable reload/import counts and `QUIT_EDITOR requested`. |
| Performance and optimization | Performance, profiling, diagnostics, telemetry, perception, or optimization-readiness | Start with `C:\UE\T66\PerformanceSystem\PERFORMANCE_SYSTEM_AGENTS.md`; keep contracts under `PerformanceSystem\` and runtime code under `Source\T66\PerformanceSystem\`. | PerformanceSystem outputs, logs, or explicit skipped-verification note. |

## 5. Environment And Repo Operations

### Standalone Shortcut

- After any change that affects the playable standalone build, refresh the staged standalone build and verify the taskbar `T66 Standalone.lnk` shortcut points to `C:\UE\T66\Saved\StagedBuilds\Windows\T66\Binaries\Win64\T66.exe`.

### Unreal Viewport Screenshot

- For T66 visual verification, a computer-wide desktop screenshot is not valid proof unless the user explicitly asks for one.
- Use Unreal-owned capture paths so the screenshot is written by the running game viewport.
- When a requested visual state has no existing capture route, add or document a targeted Unreal automation hook first, then capture through Unreal.
- Prefer synchronized structural evidence with `-T66AutoDumpScreen=<path>` or `-T66AutoDumpWidget=<Target>:<path>` whenever UI/layout verification is part of the task.

### Version, Commit, And Push

- `main` is the only normal development branch. Do not create version branches such as `alpha-0.8`, `main/alpha-0.8`, or `version-1.1` unless the user explicitly asks for a separate branch.
- When the user names a new version, treat that value as both the Unreal game version and the GitHub repo version. Update `ProjectVersion` in `Config/DefaultGame.ini` and use the same exact value for the GitHub tag/release naming unless the user explicitly asks for a different split.
- Version snapshots are Git tags on `main`. After intended changes are committed and pushed to `origin/main`, create and push the version tag from the committed `main` tip.
- When the user says "commit and push", interpret it as: commit approved changes to `main`, push `main`, create and push the next version tag, and verify the working tree is clean afterward.
- A commit/push task is not complete until `main` equals `origin/main` and there are no remaining tracked changes. If tracked changes remain, classify each as commit, restore, ignore/untrack, or explicitly deferred.
- Never use blanket discard, reset, or clean commands to make the tree clean unless the user explicitly approves that destructive cleanup.

### Git LFS Status

- Avoid broad `git status`, `git diff`, or similar Git scans over Unreal binary asset folders such as `Content/`, `SourceAssets/`, and staged build outputs unless the task specifically requires that scope.
- Prefer narrow path checks against the specific files being inspected. Broad scans can spawn many `git-lfs.exe` workers that hash large `.uasset` or generated asset files, saturating disk I/O and freezing the desktop.
- If a broad Git/LFS scan is necessary, warn the user first and explain that it may temporarily hammer disk. Treat high disk usage with many `git-lfs.exe` processes and little or no network activity as local LFS hashing/comparison, not a push or pull.

### Report Artifact Routing

- Store agent-authored reports, handoff packets, proof summaries, review packets, cleanup manifests, and temporary report runs under `Reports/`, following `Reports/AGENTS.md`.
- Use `Audit/` only for user-requested audits or existing audit lifecycle workflows. Do not put ordinary reports there.
- Report-only ToonStyle artifacts belong under `Reports/ToonStyle`, not under production ToonStyle folders.
- Raw report/proof run folders expire after 15 days. Delete whole run folders only after confirming a durable summary exists outside the raw folder and no active references still point at that run. New raw run folders should include `.report-run.json` with `expiresAfterDays: 15` for unambiguous future cleanup.

### Script Lifecycle

- Keep reusable master scripts tight and documented. Delete task-specific scripts after the task is accomplished, but first move durable process improvements into the relevant master script, README, or process doc.
