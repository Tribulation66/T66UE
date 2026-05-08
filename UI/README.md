# T66 UI Reference Workspace

The active prompt is:

```text
C:\UE\T66\UI\MASTER_REFERENCE_UI_GENERATION_PROMPT.md
```

Use that single master prompt for every screen, modal, and screen state. Fill in the target fields inside the prompt before starting a new chat.

Supporting instructions:

```text
C:\UE\T66\UI\Reference\SCREEN_MODAL_TASK.md
C:\UE\T66\Docs\UI\UI_GENERATION.md
C:\UE\T66\UI\SCREEN_WORKFLOW.md
```

Reference screenshots live under:

```text
C:\UE\T66\UI\generation\ReferenceFullScreens_2026-05-02_inventory_locked\
C:\UE\T66\UI\screens\<target>\reference\
```

Runtime assets belong under target-owned folders:

```text
C:\UE\T66\SourceAssets\UI\Reference\Screens\<ScreenName>\
C:\UE\T66\SourceAssets\UI\Reference\Modals\<ModalName>\
```

Old per-screen copy-paste prompt bundles were archived under:

```text
C:\UE\T66\UI\archive\PromptFiles_PreMaster_*
```

Each target owns its runtime image assets. If a shared-looking piece is needed for a target, duplicate it into that target's runtime folder unless the current task explicitly includes shared chrome.

Individual screen agents should use working captures from the local development executable and loose `SourceAssets` while iterating. They should not run full UAT/stage/cook/package for each target. Final packaged verification belongs to the coordinating pass after target work is finished.
