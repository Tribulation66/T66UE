# T66 Performance and Cross-Platform Readiness Audit - 2026-05-18

This folder contains the read-only audit report set prepared for Claude's two-track optimization planning:

- Low-end PC performance baseline and remediation planning.
- Cross-platform foundation for Steam Deck and macOS, with Steam Deck as the higher priority.

The audit was static/source-backed unless a file artifact already existed in the repo. No runtime editor session, game launch, build, cook, staged refresh, or code/data edit was performed during the audit. Runtime-only items are marked explicitly.

## Reports

- `01_engine_build_configuration.md`
- `02_project_settings_scalability.md`
- `03_tick_cpu_cost.md`
- `04_rendering_assets.md`
- `05_ui_performance.md`
- `06_memory_loading.md`
- `07_profiling_artifacts.md`
- `08_steam_deck_readiness.md`
- `09_macos_readiness.md`
- `10_repo_pipeline_state.md`
- `11_final_open_question.md`

## Audit Method

Evidence came from current files under `C:\UE\T66`, installed UE 5.7 plugin descriptors under `C:\Program Files\Epic Games\UE_5.7`, targeted `rg`/PowerShell scans, selected binary-string reads of `.uasset` metadata, existing material stats CSV files, and a `git status --porcelain -uno` summary.

The folder name uses `5-18 performance audit` because Windows cannot use `/` inside a single folder name.

