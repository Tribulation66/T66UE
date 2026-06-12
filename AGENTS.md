# AGENTS.md

## 1. Fast Start

- Treat this file as the root router for `C:\UE\T66`.
- Do not use native goal tools for T66 work: no `/goal`, `set_goal`, `create_goal`, `update_goal`, `get_goal`, or equivalent native goal APIs. If the host shows goal context, ignore it for scope, stopping, continuation, blocking, or completion.
- Default to normal single-agent work. In Codex, Codex handles the request. In Claude Code, Claude handles the request. Do not invoke the other model unless the user explicitly asks for Claude/Codex validation or direct Claude work.
- Do not read `.t66\validator-state.json` at startup. Claude/Codex validation is opt-in per request only; see `OPTIONAL_VALIDATOR_PROTOCOL.md` when the user explicitly asks to validate with another model.
- Start from live repo state, current folder instructions, current assets, current scripts, and current machine state. Do not answer from stale docs or memory when the live project can be checked.
- Keep a lightweight task contract internally:

```text
Working task:
Scope:
Stop condition:
```

Print the contract only when it helps clarify scope. Use it to decide what to inspect, what can change, and what verification proves the request is done.
- User constraints, planning-only boundaries, and repo instructions override convenience. If the user changes scope, replace the task contract and discard stale assumptions.
- For each completed change, report the exact verification performed, or clearly state why verification was skipped.

## 2. Discovery

- Infer the owning folder/system from the request. The user may describe the desired result rather than a path.
- Before editing files or running workflow commands, read the relevant folder `*_AGENTS.md` when one exists.
- If no folder router exists, read the nearest `README.md` and any obviously relevant instruction doc. Report a missing router only when it matters.
- If the task crosses folders, read each relevant folder router and follow the most specific instruction.
- Before changing a folder, read its `pending_issues_*.md` file when present so you do not duplicate or contradict prior decisions.
- For model, material, world-visual, or UI-brand direction questions, also read `ART_DIRECTION.md`.

## 3. Planning And Decisions

- Ask concise clarification only when ambiguity would change scope, ownership, acceptance criteria, risk, verification, or whether files should be edited.
- If a clarification is optional, say the user may reply `no clarification needed` to proceed on stated assumptions.
- Before implementation, make a proportional plan for changes to code, content, scripts, docs, config, build outputs, releases, or workflow rules. Ordinary narrow fixes do not need packet ceremony.
- When only the user can decide how the task proceeds, stop immediately after presenting the decision and choices. For durable work, save the gate as `Reports/AgentReviews/<TaskSlug>/decision_block.md`; on continuation before the user answers, only reference that decision block and restate the choices.

## 4. Process Triggers

Most prompts use the normal single-agent path. Load extra process docs only when the request matches the trigger.

| Trigger | Read / follow |
|---|---|
| Claude/Codex validation or direct Claude work explicitly requested | `OPTIONAL_VALIDATOR_PROTOCOL.md` |
| Solved-category visual/media/import/animation/audio/generated-content/VFX work | `PROVEN_PROCESS_FIDELITY.md` |
| Named T66 process area such as VFX, UI fidelity, staged standalone, data reload, performance, lifecycle, image generation, Pixal3D, or video generation | `PROCESS_REGISTRY.md`, then the owning folder docs |

Do not run PPF, Claude validation, broad build/stage scripts, or editor automation just because a new agent started. Use them only when their trigger is present.

## 5. Core Safety Rules

- Do not silently swap an accepted process for a faster or simpler method when the method matters to the result.
- Do not make destructive, credential, billing, Git/LFS, release, broad cleanup, migration, production-asset, Unreal/editor automation, or hard-to-reverse changes without clear scope.
- Do not claim current compile, run, capture, test, editor, or gameplay proof unless it was actually attempted in the current task.
- For T66 visual verification, a computer-wide desktop screenshot is not valid proof unless the user explicitly asks for one. Use Unreal-owned capture paths when visual proof matters.
- When making changes, document out-of-scope problems encountered in the relevant `pending_issues_<foldername>.md` only when they are real issues and relevant to the touched area.
- If the same command, build, test, or runtime path fails twice with the same error signature, stop repeating similar fixes. Check repo-local docs/logs, identify 3-5 plausible fixes, choose the smallest repo-appropriate solution, and report the options considered.

## 6. Environment And Repo Operations

### Standalone Shortcut

- After any change that affects the playable standalone build or taskbar shortcut, run `Scripts/StageStandaloneBuild.ps1`. It builds, cooks, stages, and refreshes `T66 Standalone.lnk` to `C:\UE\T66\Saved\StagedBuilds\Windows\T66\Binaries\Win64\T66.exe`.
- Use `Scripts/RunStagedBuildReadinessGate.ps1` only when app-booting smoke verification is explicitly required.
- For smoke-only reruns against an already-current staged executable, use `Scripts/RunPreReleaseSmokeSuite.ps1`.

### Unreal Viewport Screenshot

- Use Unreal-owned capture paths for gameplay/UI/VFX proof.
- When no capture route exists for the requested state, add or document a targeted Unreal automation hook first, then capture through Unreal.
- Prefer synchronized structural evidence with `-T66AutoDumpScreen=<path>` or `-T66AutoDumpWidget=<Target>:<path>` whenever UI/layout verification is part of the task.

### Version, Commit, And Push

- `main` is the normal development branch. Do not create version branches unless the user explicitly asks.
- When the user names a new version, use the exact value for both `ProjectVersion` in `Config/DefaultGame.ini` and the Git tag/release name unless the user asks for a split.
- When the user says `commit and push`, commit approved changes to `main`, push `main`, create and push the version tag when applicable, and verify the working tree is clean afterward.
- Never use blanket discard, reset, or clean commands unless the user explicitly approves that destructive cleanup.

### Git LFS Status

- Avoid broad `git status`, `git diff`, or similar Git scans over Unreal binary asset folders such as `Content/`, `SourceAssets/`, and staged outputs unless the task requires that scope.
- Prefer narrow path checks for the files being inspected. Broad scans can spawn many `git-lfs.exe` workers and saturate disk I/O.

### Report Artifact Routing

- Store agent-authored reports, handoff packets, proof summaries, review packets, cleanup manifests, and temporary report runs under `Reports/`, following `Reports/AGENTS.md`.
- Use `Audit/` only for user-requested audits or existing audit workflows.
- Raw report/proof run folders expire after 15 days. Delete whole run folders only after confirming a durable summary exists outside the raw folder and no active references still point at that run.

### Script Lifecycle

- Keep reusable master scripts tight and documented. Delete task-specific scripts after the task is accomplished, but first move durable process improvements into the relevant master script, README, or process doc.

### Background Task Discipline

- Default to foreground execution with an explicit timeout. Use background execution only when the work genuinely must overlap with other work in the same task, and state the stop condition at launch time.
- Before ending a task, stop every background task you started once it has served its purpose. Never end a turn with an orphaned watcher, poller, or worker still running.
- Exemption: background work the user explicitly asked to outlive the task (e.g. overnight runs). Name any such surviving task in the final report so the user knows it is still running and why.
- When a background command must spawn a long-lived child process (Unreal editor, staged game, local server), launch the child detached with output redirected to a file so the shell task completes immediately instead of hanging on inherited console handles.
- This cleanup applies only to processes the current agent started. Never kill Codex workers, the user's editor sessions, or any process another tool owns.
- The user has a taskbar badge alert that fires on every turn end (Stop hook). If you end a turn that will auto-resume without user input (e.g. a background build will re-wake you and the user's request is not yet fully complete), suppress that one badge by running `New-Item -ItemType File "$env:LOCALAPPDATA\claude-code-badge-suppress" -Force | Out-Null` as the last action before ending the turn. Never suppress when the request is fully complete or when you are blocked waiting on the user.
