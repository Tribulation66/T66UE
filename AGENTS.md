# AGENTS.md

## Standalone Shortcut Rule

- After any change that affects the playable standalone build, refresh the staged standalone build and verify the taskbar `T66 Standalone.lnk` shortcut points to `C:\UE\T66\Saved\StagedBuilds\Windows\T66\Binaries\Win64\T66.exe`.

## Version Naming Rule

- When the user names a new version, treat that value as both the Unreal game version and the GitHub repo version. Update `ProjectVersion` in `Config/DefaultGame.ini` and use the same exact value for the GitHub branch/tag/release naming unless the user explicitly asks for a different split.

## Script Lifecycle Rule

- Keep reusable master scripts tight and documented. Delete task-specific scripts after the task is accomplished, but first move any durable process improvements into the relevant master script, README, or process doc.

## PerformanceSystem Rule

- For performance, diagnostics, profiling, telemetry, perception, or optimization-readiness work, start with `C:\UE\T66\PerformanceSystem\PERFORMANCE_SYSTEM_AGENTS.md`; keep root contracts/schemas under `PerformanceSystem\` and Unreal runtime code under `Source\T66\PerformanceSystem\`.

## Git LFS Status Rule

- Avoid broad `git status`, `git diff`, or similar Git scans over Unreal binary asset folders such as `Content/`, `SourceAssets/`, and staged build outputs unless the task specifically requires that scope.
- Prefer narrow path checks against the specific files being inspected. Broad scans can spawn many `git-lfs.exe` workers that hash large `.uasset` or generated asset files, saturating disk I/O and freezing the desktop.
- If a broad Git/LFS scan is necessary, warn the user first and explain that it may temporarily hammer disk. Treat high disk usage with many `git-lfs.exe` processes and little or no network activity as local LFS hashing/comparison, not a push or pull.

## Goal Translation Rule

- Before acting, derive the current working goal in one sentence. Use it to decide:
  1. what files/systems to inspect,
  2. what changes are in scope,
  3. what verification proves the request is done.
- If the user changes scope, replace the working goal and discard stale assumptions.
- The working goal must not override explicit user constraints, planning-only boundaries, or repository instructions.

## Folder Instruction Discovery Rule

- Before acting, infer which project folder owns the user's request. The user may describe the task by goal rather than by folder name.
- Use task wording, repo search, paths, READMEs, and existing docs to identify the responsible folder.
- Read that folder's `*_AGENTS.md` before editing files or running workflow commands.
- If the task crosses folders, read each relevant folder agent file and follow the most specific applicable instructions.
- Folder agent files are routers. They point to the required instruction files; they do not replace those files.
- If no folder agent exists, read the nearest `README.md` and relevant instruction docs, proceed conservatively, and report the missing router as a documentation gap.

## Parallel Delegation Rule

- When the current working goal can be completed faster by splitting independent work, use available sub-agent or delegation tools instead of doing every task serially.
- Delegate only concrete, bounded work that can run in parallel without blocking the critical path, such as targeted repo exploration, reference gathering, disjoint implementation work, or focused verification.
- Before delegating, define each agent's objective, owned files/systems, constraints, and expected output. For code edits, keep write scopes disjoint and tell agents not to revert or overwrite user changes or peer work.
- Keep moving on the local critical path while delegated work runs. Do not spawn agents for trivial, tightly coupled, unclear, or high-risk tasks where coordination would cost more than it saves.
- The lead agent remains responsible for integrating results, resolving conflicts, and reporting verification evidence. Delegation does not weaken any explicit user constraint, repository rule, or required verification gate.

## Verification Evidence Rule

- For each completed change, report the exact verification performed, or state clearly why verification was skipped. Treat compile/build results, staged standalone checks, logs, screenshots, and runtime smoke tests as evidence when they are relevant to proving the request is done.

## Repeated Error Research Rule

- If the same command, build, test, or runtime path fails twice with the same error signature, stop repeating similar fixes.
- Before attempting a third fix, research the issue. Check repo-local docs/logs first, then use web research when the cause is not already clear.
- Identify 3-5 plausible fixes, choose the smallest repo-appropriate solution, implement it, and report the options considered plus verification evidence.
- Prefer official docs, engine/source references, issue trackers, and highly relevant forum threads over generic answers.

## Pending Issues Tracking

When making changes to the codebase, any problem you encounter that is out of scope for your current pass -- broken systems, schema redundancies, missing classes, hardcoded assumptions, design debt, etc. -- must be documented in a `pending_issues_<foldername>.md` file located in the same folder as the affected code or data.

Format:
- One section per issue with a heading
- Severity tag: [Blocker] / [Major] / [Minor]
- What's wrong: brief description with file references
- Why it's out of scope now: reason
- What fixing it would entail: scope estimate

Before working in any folder, read its `pending_issues_*.md` file (if present) so you don't duplicate or contradict prior agents' decisions.

## Pixal3D ToonStyle Production Import Rule

- Pixal3D is production-cleared for T66 model replacement work. Do not treat it as research-only when the task is to regenerate or replace interactables, props, mobs, or ToonStyle production assets.
- For any Pixal3D asset that will enter playable content, use the manifest-driven ToonStyle production workflow documented in `Model Generation/Instructions/09_PIXAL3D_TOONSTYLE_PRODUCTION_IMPORT_INSTRUCTIONS.md`.
- Do not use legacy one-off Pixal3D import scripts or manually assign ToonStyle materials. The production path must run the Pixal3D generation settings, Blender ToonStyle foundation pipeline, Unreal importer, and hard validators so Tint, outline, close-the-gap B, inner-line texture, import settings, and verification JSON are all present.
- The production Pixal3D face target is `X-Decimation=200000` with remesh enabled. Lower-decimation or no-remesh output is a fallback only when the server reports it, and it must be surfaced in the replacement manifest/report rather than silently accepted.
- If a generated asset affects the playable standalone build, refresh the staged build and verify the taskbar standalone shortcut after import.

## UI Reference Fidelity Rule

- When implementing or editing a UI screen from a reference image, follow the loop defined in `C:\UE\T66\UI\Instructions\UI_FIDELITY_LOOP_INSTRUCTIONS.md`.
- Specifically:
  1. Run Step 0 (legacy chrome cleanup) before any flat construction. The screen's reachable code must produce no matches against the validation regex in Section 5.1 of the loop doc.
  2. Tag every named element constructed via `FT66FlatStyle` helpers per the convention in Section 3.3 of the loop doc.
  3. Iterate Steps 1-7. The screen is not done until the verification report shows zero FAIL items and either zero UNSURE items or all UNSURE items previously accepted as content deltas.
  4. Terminate per Section 8 of the loop doc. On ESCALATE, produce the Pablo review packet and stop.
  5. Compile success and "looks roughly right" are necessary but not sufficient. The `VerifyUIFidelity` report is the gate.
- Do not declare a UI migration complete without running the loop.
- Do not skip Step 0.
- Do not resume an in-progress migration without re-running Step 0's audit against current state.
