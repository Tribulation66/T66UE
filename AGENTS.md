# AGENTS.md

## 1. Project Contract

- Treat this file as the root process router for `C:\UE\T66`. It defines global behavior, accepted process classes, and where to find deeper folder-owned instructions.
- Before answering or acting on every new user request or question, derive the current working goal in one sentence. Use the native goal/set_goal mechanism when available; otherwise state the working goal in the conversation.
- Use the working goal to decide what files/systems to inspect, what changes are in scope, and what verification proves the request is done.
- Start from the live repo, current folder instructions, current assets, current scripts, and current machine state. Do not answer from stale docs or memory when the live project can be checked.
- Default scope excludes Mini/minigame systems. Unless the user explicitly names Mini, minigames, a specific minigame, or a Mini-owned path, do not inspect, change, recommend, capture, validate, or include Mini/minigame code, UI, assets, docs, checklists, or generated outputs. Do not treat unrelated terms such as minimap or mini-stat as Mini scope. If a task appears impossible without Mini/minigame work, ask before including it.
- User constraints, planning-only boundaries, and repository instructions override convenience. If the user changes scope, replace the working goal and discard stale assumptions.
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

### Goal Clarification

- After deriving the working goal, decide whether the request is clear enough to plan safely.
- If ambiguity would change scope, ownership, acceptance criteria, risk, verification, or whether files should be edited, ask concise clarifying questions and wait for the user's answer.
- Every clarification step must make the user's opt-outs explicit: the user may say `no clarification needed` to proceed on stated assumptions, or `skip Claude review` to bypass the Claude cross-review for that request.
- If no clarification is needed, state the safe assumptions and continue to planning and Claude review by default unless the user has already opted out.

### Folder Instruction Discovery

- Before acting, infer which project folder owns the request. The user may describe the task by goal rather than by folder name.
- Use task wording, repo search, paths, READMEs, and existing docs to identify the responsible folder.
- Read that folder's `*_AGENTS.md` before editing files or running workflow commands.
- If the task crosses folders, read each relevant folder agent file and follow the most specific applicable instructions.
- Folder agent files are routers. They point to required instruction files; they do not replace those files.
- If no folder agent exists, read the nearest `README.md` and relevant instruction docs, proceed conservatively, and report the missing router as a documentation gap.

### Implementation Planning

- Before implementation, create an implementation plan for every request that may change code, content, scripts, docs, configuration, build outputs, releases, or workflow rules.
- For question-only requests, create a short answer plan when repo inspection, external review, or non-trivial reasoning is needed.
- The plan must include: clarified goal, user constraints, files/folders/systems to inspect or edit, applicable `*_AGENTS.md` and instruction files, intended edit scope, out-of-scope items, risks, rollback or revert considerations, and verification evidence required before completion.
- Trivial read-only commands and direct factual answers may use a compact plan, but must still respect goal clarification, review defaults, and the review greenlight gate before any substantive output unless the user opts out.

### Claude Cross-Review

- Claude review is the default for every request or question unless the user explicitly says `skip Claude review`, the Claude CLI is unavailable, or the task is impossible to review without credentials or context the agent cannot access.
- Claude must be used through the local Claude Code CLI authenticated to the user's Claude subscription, not through Anthropic API billing.
- Before running Claude review, verify that `ANTHROPIC_API_KEY` is not set in the active environment. If it is set, stop and ask the user how to proceed rather than risking API charges.
- Use `Scripts\Invoke-ClaudePlanReview.ps1` for normal Claude reviews. The helper must prefer the local Claude Code CLI, not the Windows desktop app shim, and must retry timeouts with a fresh CLI process before treating the review as failed.
- If Claude Code is unavailable because the local subscription session limit has been reached, the CLI is missing, the local subscription login is unavailable, or a comparable Claude CLI availability failure occurs, use `Scripts\Invoke-CodexPlanReview.ps1` as the fallback reviewer instead of blocking the task. This standing fallback is pre-approved by the user for Claude session-limit cases. Malformed Claude verdict output is not a Claude availability failure, is not a greenlight, and is not a Codex fallback trigger; fail closed and ask the user how to proceed.
- The Codex fallback reviewer must be a separate local Codex CLI process, not the active Codex reasoning pass. It must receive the same read-only review packet expected for Claude, run through the project helper, and act only as reviewer: no edits, no implementation, no command-running for the plan.
- A Codex fallback greenlight exists only when the saved fallback review artifact is bound to the current working goal and output scope, and its first non-empty line is exactly `Verdict: APPROVE`. Missing, malformed, quoted, heading-prefixed, body-quoted, REVISE, or BLOCK verdicts are not greenlights.
- When Codex fallback review is used, report that Claude was unavailable, name the fallback review artifact path, and treat unresolved Blocker or Major objections exactly like Claude objections.
- Send the reviewer a read-only review packet containing the working goal, clarified assumptions, applicable repo instructions, PPF check when applicable, proposed plan, affected files/systems, known risks, and verification gates.
- Before sending the packet, Codex must record its own evidence-backed opinion or plan first. External review is a cross-check against Codex's reasoning, not a substitute for Codex doing the analysis.
- The reviewer's role is reviewer, not implementer. Ask it to identify flawed assumptions, missing files, unsafe scope, inadequate verification, contradictions with repo instructions, unanswered goal questions, and lazy or careless reasoning by the implementing agent.
- Codex must revise the plan or explicitly reject each reviewer objection with repo-grounded reasoning. When reviewer and implementer disagree on a meaningful point, Codex must compare both positions, identify which side was wrong or incomplete and why, and carry forward the best-supported synthesis.
- Repeat review until there are no unresolved Blocker or Major objections, all external reviewers are unavailable, the user opts out, or Codex and the reviewer identify a concrete unresolved decision that only the user can make.
- Persistent non-convergence is itself a blocked decision, not a numeric pass cap. If the same unresolved Blocker/Major issue set recurs across consecutive reviewed revisions after Codex has made a good-faith revision, and Codex has no new repo-grounded counter, implementation alternative, or verification path to try, Codex must surface that exact decision to the user instead of continuing silently. Productive iteration continues until the problem is solved.
- Before Codex gives substantive user-facing output, the exact output scope must be greenlit by Claude or by the Codex fallback reviewer when fallback use is allowed. Substantive output includes recommendations, implementation plans presented as accepted, source/video selections, visual/media/artifact handoffs, final answers, acceptance claims, and done reports, including trivial-task completion reports.
- A Claude greenlight exists only when the saved review artifact is bound to the current working goal and output scope, and its first non-empty line is exactly `Verdict: APPROVE`. Missing, malformed, quoted, heading-prefixed, body-quoted, REVISE, or BLOCK verdicts are not greenlights. A greenlight for one scope cannot be reused for a different answer, recommendation, source selection, artifact handoff, final answer, or completion report.
- Before the greenlight, Codex may provide only concise procedural status needed to run review or concise clarifying questions needed to resolve scope, ownership, acceptance criteria, risk, credentials, or whether to opt out of review. Reporting that a reviewed plan received `Verdict: APPROVE` and asking for user go-ahead is permitted procedural status when bound to that same review artifact.
- If Claude and the Codex fallback reviewer are both unavailable, cannot access required context, or cannot review without credentials the agent does not have, Codex must say the greenlight cannot be satisfied and ask whether the user wants to continue without external review. Codex must not treat its own confidence as a substitute for the greenlight.
- Before implementation begins, report the conclusion Codex and the reviewer reached, including any accepted caveats or deferred minor issues, then wait for the user's explicit go-ahead.
- Question-only substantive answers still require the review greenlight gate unless the user explicitly opts out or approves continuing when all external reviewers are unavailable.

### Delegation And Error Handling

- When the current working goal can be completed faster by splitting independent work, use available sub-agent or delegation tools instead of doing every task serially.
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
| Claude cross-review | Any non-trivial request or question unless the user opts out | Use local Claude Code CLI by default. Verify no `ANTHROPIC_API_KEY` in Process/User/Machine. Send read-only review packet; do not use Anthropic API billing. If Claude is unavailable due to session limit or comparable CLI availability failure, use `Scripts\Invoke-CodexPlanReview.ps1` as the separate local Codex CLI fallback reviewer. | Report the greenlight artifact path, output scope, reviewer used, and any caveats before implementation; a substantive output is greenlit only by an exact `Verdict: APPROVE` artifact bound to that scope. |
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

### Script Lifecycle

- Keep reusable master scripts tight and documented. Delete task-specific scripts after the task is accomplished, but first move durable process improvements into the relevant master script, README, or process doc.
