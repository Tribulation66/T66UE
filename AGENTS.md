# AGENTS.md

## 1. Project Contract

- Treat this file as the root process router for `C:\UE\T66`. It defines global behavior, accepted process classes, and where to find deeper folder-owned instructions.
- Before answering or acting on every new user request or question, derive a plain task contract. Do not call native goal tools for T66 work: do not create, update, complete, block, or query goals (`/goal`, `set_goal`, `create_goal`, `update_goal`, `get_goal`, or equivalent native goal APIs) as part of repo process. If the host app displays, injects, or exposes goal context anyway, treat it as non-authoritative telemetry only and ignore it for task control; it does not control scope, stop conditions, continuations, blocking, completion, or whether work may continue.
- The task contract is the active scope source. Keep it lightweight and prompt-native:

```text
Working task:
Scope:
Stop condition:
```

Use the task contract to decide what files/systems to inspect, what changes are in scope, and what verification proves the request is done.
- Start from the live repo, current folder instructions, current assets, current scripts, and current machine state. Do not answer from stale docs or memory when the live project can be checked.
- For model, material, world-visual, or UI-brand direction questions, also read `C:\UE\T66\ART_DIRECTION.md`. It declares FriendSlop as the active 3D/world direction, keeps UI FriendslopStyle separate from the 3D rubber-material contract, and points deprecated ToonStyle/RetroFX art-direction docs to `Archive/`.
- Default to normal single-agent operation. If the user is in Codex, Codex acts normally; if the user is in Claude Code, Claude acts normally. Do not invoke the other model by default.
- After deriving the task contract, read `C:\UE\T66\.t66\validator-state.json` when it exists to check the optional `validatorMode` setting and named validator. Treat missing or malformed state as `validatorMode: off` unless the user explicitly asked for a validator. The tray runtime file under `C:\Users\DoPra\AppData\Local\T66UsageTray` is a UI mirror, not the canonical agent state.
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
- Every clarification step must make the user's clarification opt-out explicit: the user may say `no clarification needed` to proceed on stated assumptions. This does not bypass an explicitly requested or persistently enabled validator.
- If no clarification is needed, state the safe assumptions and continue through the normal single-agent path, or through the opt-in validator loop only when it is engaged.

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

### Opt-In Validator Loop

- The default process is normal single-agent work by the model the user is currently using. No cross-model validator runs unless validation is engaged.
- Engage a validator only when `validatorMode` is `on` in `.t66\validator-state.json`, or when the user asks for a per-request validator such as `implement validator Claude` or `implement validator Codex`.
- A per-request validator command is ephemeral. It applies only to the current question, stuck step, failed approach, or specific problem being debugged. It must not write persistent validator state.
- Persistent commands such as `turn on validator setting` and `turn off validator setting` update `validatorMode` through `Scripts\Set-T66ValidatorMode.ps1`. With validator mode on, future T66 prompts use the named validator until the setting is turned off.
- The validator exists to help get unstuck, find a solution to a problem one model could not solve alone, catch missed constraints, or add independent scrutiny to high-risk work. It is not the baseline process for ordinary prompts.
- When a validator is engaged, the active model still owns the real work: discovery, planning, edits, command/editor work, evidence collection, and final synthesis. The validator provides an independent answer or targeted cross-review, not a second implementation owner by default.
- Validator helper results stay binary: `OK` when the models can resolve the prompt internally, or `NEEDS_USER` when the user's attention is required for a decision, approval, hard blocker, or unavailable required tool. The parser may infer that result from clear wording and must not fail just because Claude wrote a short sentence before the result line. Ordinary fixable review findings do not become a blocking result; the active model handles them before answering.
- If the requested validator tool is unavailable, report it explicitly only when it affects the requested validator work. Otherwise proceed with normal single-agent work and do not claim cross-model validation.
- Keep these safety stops even when no validator is engaged: ask the user when only the user can decide; do not make destructive, credential, billing, Git/LFS, release, broad cleanup, migration, production-asset, Unreal/editor automation, or hard-to-reverse changes without clear scope; do not claim current proof that was not actually run; do not let Claude make mutating changes from Codex without Codex-approved scope.
### Optional Validator Protocol

- For requests that explicitly use the Claude/Codex validator stack, follow `OPTIONAL_VALIDATOR_PROTOCOL.md`. That file owns the opt-in loop, validator setting, Claude helper boundaries, and proof ownership.
- Core invariant when validation is engaged: the validator produces an independent repo-grounded answer or targeted cross-review; the active model produces its own work or draft; both outputs are checked for oversights; the final answer synthesizes the result.
- The Validator is the named validator from `.t66\validator-state.json` unless the user explicitly names a different reviewer. Direct per-request commands `implement validator Claude` and `implement validator Codex` engage that validator for the current request only. Direct persistent commands `turn on validator setting` and `turn off validator setting` update `validatorMode` through `Scripts\Set-T66ValidatorMode.ps1`.
- Before running Claude as a direct worker or validator, verify that `ANTHROPIC_API_KEY` is not set in Process, User, or Machine scope. If it is set, stop and ask the user how to proceed rather than risking API charges.
- Claude direct-work and validator runs must use the local Claude Code CLI authenticated to the user's Claude subscription. Use `Scripts\Invoke-ClaudeDirectRead.ps1` for explicit Claude direct work and `Scripts\Invoke-ClaudePlanReview.ps1` for Claude independent-answer and cross-review passes when Claude is requested as validator. Do not use Claude `plan` permission mode for mutating direct work. For an approval-free read-only direct run, use `Scripts\Invoke-ClaudeReadOnlyOperator.ps1`; pass `-Preflight` to helpers that support it to print effective configuration without invoking Claude.
- Verification freshness is a hard rule: when the user explicitly asks for current compile, run, capture, test, or editor verification, the active model must attempt that exact current verification unless it is physically impossible. Recent or prior evidence cannot satisfy an explicit current-verification request; if it cannot be run, say so explicitly instead of substituting older evidence. See `OPTIONAL_VALIDATOR_PROTOCOL.md`.
- Broad implementation tasks may be split into phases when that is the clearest way to keep scope, proof, and rollback understandable. Do not phase work just to satisfy ceremony.
- When the user explicitly asks Claude to do mutating work from Codex, Codex must approve or reject the requested change scope before Claude makes changes. Approved Claude direct-work runs may use the full Claude Code tool surface through `Scripts\Invoke-ClaudeDirectRead.ps1 -ToolProfile FullOperator`, including edits, shell commands, and configured MCP/editor tools, inside the Codex-approved scope.
- Proof-bearing Claude direct work (tasks whose acceptance needs produced build logs, commandlet markers, runtime/editor captures, gameplay proof, visual judgment, or multiple proof classes) must use a tool path that can actually produce the evidence. Claude-produced proof is evidence, not final acceptance; the active model validates the actual evidence before reporting completion. `OPTIONAL_VALIDATOR_PROTOCOL.md` owns the proof-routing and final-proof-owner rules.
- Claude artifacts and Claude-made changes are not automatic greenlights. In the active Codex workspace, Codex remains responsible for the task contract, Codex approval when required, validation of actual changes, excluded operations, final verification review, and the user-facing completion report unless the user explicitly changes that.
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
| Optional validator | `validatorMode` is `on`, the user says `implement validator Claude` or `implement validator Codex`, or the user otherwise asks for cross-model validation/review on a specific stuck step or unsolved problem | Follow `OPTIONAL_VALIDATOR_PROTOCOL.md`. The named validator is used unless the user explicitly names a different reviewer. The validator creates an independent repo-grounded answer or targeted cross-review, then the active model incorporates valid corrections. When Claude validates Codex, use local Claude Code CLI through `Scripts\Invoke-ClaudePlanReview.ps1`. Verify no `ANTHROPIC_API_KEY` in Process/User/Machine, and do not use Anthropic API billing. Do not use hard review-depth categories or packet ceremony for ordinary answers. | Report validator mode, validator used if any, whether validation was persistent or per-request, output scope, caveats, and verification performed or skipped. |
| Claude tool access | Claude needs direct repo/file, shell, Blender, Unreal, Niagara, or editor visibility as an explicit direct worker or validator | Use `Scripts\Invoke-ClaudeDirectRead.ps1`. For Claude mutating work from Codex, use `-ToolProfile FullOperator` with a valid Codex approval artifact and non-plan permission mode. `FullOperator` is a helper profile name, not a standing T66 role. The profile may use edits, shell commands, and configured MCP/editor tools inside the approved scope. Use user-scoped Blender MCP only after smoke verification. Use Unreal-owned capture/dump scripts for Unreal/Niagara evidence; do not accept desktop screenshots or raw GUI observation as proof. | Claude helper artifact path, Codex approval artifact path for mutating runs, actual-change validation, Blender MCP list/get output when used, and Unreal-owned artifact paths for visual/editor evidence. |
| Image generation | New bitmap visuals, mockups, sprites, reference images, or image edits | Use the approved built-in `imagegen` / account-backed image generation path. For repo-bound mockups that would clutter the main chat, use a separate local Codex CLI worker with the same account-backed imagegen process for prompt, artifact, variant, and generation management. Do not require, revive, or fall back to `OPENAI_API_KEY` API scripts when account-backed imagegen is the intended process. | Generated image artifact plus visual review; for UI prompts use the self-contained prompt contract under `UI/Reference`. |
| Unreal video and screenshots | Gameplay/UI visual proof, VFX filming, capture requests | Use Unreal-owned capture paths, not desktop screenshots. For VFX/gameplay video use `Scripts/CaptureT66GameplayVideo.ps1`; for UI use `CaptureT66UIScreen.ps1` or `CaptureT66UIWidget.ps1`. Before judging visual proof, perform a diagnostic visibility check: target size, angle, occlusion, temporal sampling, and whether the frames prove the stated gate. Diagnostic captures/contact sheets may help understand an artifact, but they do not replace a locked acceptance camera or gate unless the user explicitly approves that new gate. | Output PNG/MP4 path, frame/log evidence, visibility check result, and ffmpeg/ffprobe check when relevant. |
| Loot UI animation | Loot crate, loot chest, loot bag, loot wheel, or other post-interaction 2D/UI reward animations | Use Section 2 research-first replication plus `UI/Processes/LootUIAnimationAuthoringProcedure.md`. Video references require user-provided transcripts; non-video written references should be read directly. The primary animation carrier must be a target-owned UI surface, not a mislabeled shared card or static result. | Per-target mechanism packet, artifact parity gate, focused build, Unreal-owned capture, ffprobe/frame evidence, and anti-lookalike discriminator proof. |
| Niagara combat VFX | New combat VFX, slash, aura, projectile trail, elemental attack, or weapon effect | Use Section 2 research-first replication plus Niagara/material/texture workflow: texture masks, material animation, emitters/ribbons/sprites/meshes, timing curves, and reference/source analysis. When an effect packet defines a mockup visual target, follow the additive visual-target gate in `Gameplay/Combat/CombatVFXAuthoringProcedure.md`; the mockup controls visual direction only and cannot replace source-method mechanisms. The primary silhouette must live in the Niagara system, renderer material, renderer mesh/ribbon, or emitter logic. Do not replace with procedural C++ debug geometry, static mesh actor layers, actor-arranged point components, or static lookalike masks unless explicitly approved. Slash/trail/aura tasks require a mechanism manifest for motion, timing, material animation, and dissipation before implementation. | Inspectable Niagara/material assets plus Unreal-owned capture, visual readability review, and multi-frame evidence for temporal mechanisms. Reference `Gameplay/Combat/CombatVFXAuthoringProcedure.md` for generic authoring gates and `Gameplay/Combat/Hero1AxeVFXPlan.md` for Hero 1 axe lab work. |
| UI reference fidelity | UI screen implementation or edit from a reference image | Follow `C:\UE\T66\UI\Instructions\UI_FIDELITY_LOOP_INSTRUCTIONS.md`. Do not accept "looks roughly right" or compile success without screenshot/dump/fidelity evidence. For FriendslopStyle screens, follow `C:\UE\T66\UI\FriendslopStyle\FRIENDSLOP_STYLE_IMPLEMENTATION_INSTRUCTIONS.md`: Codex produces visual evidence for user review and reports only the wiring/functionality gate as PASS/FAIL. | FlatStyle/reference-checklist work uses the owning fidelity checklist and structural report. FriendslopStyle work uses capture/dump/contact evidence, worker records, and the wiring/functionality gate. |
| FriendSlop raw Pixal3D import | Pixal3D asset entering active FriendSlop playable content | Use `Model Generation/Instructions/11_FRIENDSLOP_RAW_PIXAL3D_IMPORT_GUIDELINES.md` for raw FriendSlop static-mesh imports and `Model Generation/Instructions/13_FRIENDSLOP_RAW_HUMANOID_RIGGING_INSTRUCTIONS.md` for physics-first humanoid rigs. Preserve the generated GLB base-color texture identity and current material bindings; the future rubber master and its lit-vs-unlit decision are a separate material-migration scope. Archived ToonStyle and QuadRetro docs under `Archive/` are historical only unless the user explicitly revives them. | Manifest/source-run evidence, Blender QA where applicable, Unreal import validation, DataTable reload/import counts, material/texture binding proof, and staged build proof if playable content is affected. |
| Lifecycle coordination | Run lifecycle, loaded run, run end, return to frontend, world transition, durable-state flush, teardown audit, runtime drain, or coordinating/overseer systems | Start with `LifecycleSystem\LIFECYCLE_SYSTEM_AGENTS.md` and `LifecycleSystem\LIFECYCLE_COORDINATOR_REGISTRY.md`. Keep `ShutdownSystem` as the quit/pre-exit owner. Prefer lightweight owner-local boundaries first; add dedicated coordinators only when ordering, flushing, or shared diagnostics prove they are needed. | Lifecycle registry/audit evidence; focused compile for runtime C++ boundary changes; staged standalone validation when playable standalone behavior changes. |
| Runtime health readiness | Runtime health, startup health, required subsystem availability, diagnostics readiness, PerformanceSystem artifact proof, packaged runtime health, runtime ownership inventory, or shared runtime gate | Start with `PerformanceSystem\PERFORMANCE_SYSTEM_AGENTS.md`, `PerformanceSystem\RUNTIME_HEALTH_GATE.md`, and `PerformanceSystem\RUNTIME_OWNERSHIP_INVENTORY.md`. Use `Scripts\RunRuntimeHealthGate.ps1` for reusable packaged startup/runtime diagnostics proof. Do not replace the full staged-readiness or pre-release smoke suites with this gate; compose them according to scope. | `Saved\RuntimeHealthGate\<timestamp>\summary.json`, runtime launch log/screenshot, child staged-readiness summary, fresh PerformanceSystem snapshot/session summary, and schema/write-queue assertions. |
| Staged standalone verification | Any change affecting the playable standalone build or taskbar shortcut | Run `Scripts\StageStandaloneBuild.ps1` to refresh the staged standalone and shortcut (build + cook + stage; launches nothing). Do not treat source/editor compile as proof of what the taskbar launches. Reach for `Scripts\RunStagedBuildReadinessGate.ps1` only when app-booting smoke verification is explicitly required. | `Scripts\StageStandaloneBuild.ps1`, refreshed staged exe at `C:\UE\T66\Saved\StagedBuilds\Windows\T66\Binaries\Win64\T66.exe`, verified shortcut target; smoke logs only if the readiness gate was explicitly requested. |
| Video generation | Frontend/hero-selection generated videos or existing video-generation workflow research | Start from `Video Generation` docs, manifests, job records, and validators. Do not jump straight to ad hoc ffmpeg or generation scripts. | Manifest/job counts, runtime movie paths, validator output, and encoded media evidence. |
| Data asset import/reload | Source CSV/JSON/PNG changes that back Unreal DataTables or cooked assets | Update source data, then run the owning Unreal commandlet/import script. Do not stop at file edits when runtime assets must be refreshed. | Script log markers such as DataTable reload/import counts and `QUIT_EDITOR requested`. |
| Performance and optimization | Performance, profiling, diagnostics, telemetry, perception, or optimization-readiness | Start with `C:\UE\T66\PerformanceSystem\PERFORMANCE_SYSTEM_AGENTS.md`; keep contracts under `PerformanceSystem\` and runtime code under `Source\T66\PerformanceSystem\`. Use the runtime health readiness process when the task needs packaged startup/runtime diagnostics proof. | PerformanceSystem outputs, logs, runtime health gate output, or explicit skipped-verification note. |

## 5. Environment And Repo Operations

### Standalone Shortcut

- After any change that affects the playable standalone build, run `Scripts/StageStandaloneBuild.ps1` to update the standalone. It builds the Development target, cooks, stages, and refreshes the `T66 Standalone.lnk` shortcut (project root + pinned taskbar) to `C:\UE\T66\Saved\StagedBuilds\Windows\T66\Binaries\Win64\T66.exe`. It launches nothing — this is the default whenever the goal is simply getting the change into the standalone. Do not treat a source/editor compile as proof of what the taskbar launches; the game target has separate binaries and must be staged.
- Only when you explicitly need release-candidate smoke verification — which boots the staged app repeatedly (frontend tag-click matrix, durable-save, and lifecycle gates) — run `Scripts/RunStagedBuildReadinessGate.ps1`. It stages via `StageStandaloneBuild.ps1` and then runs the pre-release smoke suite. Do not run the readiness gate just to update the standalone; reach for it only when app-booting smoke proof is the actual ask. (It is also still consumed by `Scripts/RunRuntimeHealthGate.ps1`.)
- For smoke-only reruns against an already-current staged executable, use `Scripts/RunPreReleaseSmokeSuite.ps1`. It runs the frontend tag-click matrix and, when the target build is Development/non-shipping, the durable save integrity and lifecycle transition gates. See `Scripts/README.md` for exact commands and the Shipping-build caveat.

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
- Report-only historical ToonStyle artifacts belong under `Reports/ToonStyle`, not under production folders. Deprecated ToonStyle and RetroFX/PS1 process docs live under root `Archive/` and are not active instructions.
- Raw report/proof run folders expire after 15 days. Delete whole run folders only after confirming a durable summary exists outside the raw folder and no active references still point at that run. New raw run folders should include `.report-run.json` with `expiresAfterDays: 15` for unambiguous future cleanup.

### Script Lifecycle

- Keep reusable master scripts tight and documented. Delete task-specific scripts after the task is accomplished, but first move durable process improvements into the relevant master script, README, or process doc.
