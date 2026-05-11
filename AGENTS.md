# AGENTS.md

## Standalone Shortcut Rule

- After any change that affects the playable standalone build, refresh the staged standalone build and verify the taskbar `T66 Standalone.lnk` shortcut points to `C:\UE\T66\Saved\StagedBuilds\Windows\T66\Binaries\Win64\T66.exe`.

## Version Naming Rule

- When the user names a new version, treat that value as both the Unreal game version and the GitHub repo version. Update `ProjectVersion` in `Config/DefaultGame.ini` and use the same exact value for the GitHub branch/tag/release naming unless the user explicitly asks for a different split.

## Script Lifecycle Rule

- Keep reusable master scripts tight and documented. Delete task-specific scripts after the task is accomplished, but first move any durable process improvements into the relevant master script, README, or process doc.

## Goal Translation Rule

- Before acting, derive the current working goal in one sentence. Use it to decide:
  1. what files/systems to inspect,
  2. what changes are in scope,
  3. what verification proves the request is done.
- If the user changes scope, replace the working goal and discard stale assumptions.
- The working goal must not override explicit user constraints, planning-only boundaries, or repository instructions.

## Verification Evidence Rule

- For each completed change, report the exact verification performed, or state clearly why verification was skipped. Treat compile/build results, staged standalone checks, logs, screenshots, and runtime smoke tests as evidence when they are relevant to proving the request is done.
