# T66 UI Reference Workspace

The active prompt is:

```text
C:\UE\T66\UI\MASTER_REFERENCE_UI_GENERATION_PROMPT.md
```

Use that single master prompt for every screen, modal, and screen state. Fill in the target fields inside the prompt before starting a new chat.

Supporting instructions:

```text
C:\UE\T66\UI\Instructions\UI_LAYOUT_AND_SIZING_INSTRUCTIONS.md
C:\UE\T66\UI\Instructions\UI_SCREEN_MODAL_INSTRUCTIONS.md
C:\UE\T66\UI\Instructions\UI_GENERATION_INSTRUCTIONS.md
C:\UE\T66\UI\Instructions\UI_SCREEN_WORKFLOW_INSTRUCTIONS.md
```

Reference screenshots and generated proof images are temporary review outputs. Keep accepted runtime art in `RuntimeDependencies`; do not recreate persistent `UI/generation`, `UI/screens`, `UI/Reference`, or `UI/archive` output roots.

```text
C:\UE\T66\Saved\Codex\UI\<ScreenName>\
```

Runtime assets belong under target-owned folders:

```text
C:\UE\T66\RuntimeDependencies\T66\UI\Reference\Screens\<ScreenName>\
C:\UE\T66\RuntimeDependencies\T66\UI\Reference\Modals\<ModalName>\
```

Old per-screen copy-paste prompt bundles were removed during the Alpha 0.1 cleanup. Recover them from the `v4.1` snapshot only if a historical prompt is specifically needed.

Each target owns its runtime image assets. If a shared-looking piece is needed for a target, duplicate it into that target's runtime folder unless the current task explicitly includes shared chrome.

Individual screen agents should use working captures from the local development executable and loose `RuntimeDependencies` while iterating. They should not run full UAT/stage/cook/package for each target. Final packaged verification belongs to the coordinating pass after target work is finished.

