# T66 UI Reference Workspace

The authoritative instructions are in:

```text
C:\UE\T66\Docs\UI\UI_GENERATION.md
```

This workspace now supports screen-by-screen and modal-by-modal UI generation.

Key folders:

```text
C:\UE\T66\UI\Reference\Screens\<ScreenName>\PROMPT.md
C:\UE\T66\UI\Reference\Modals\<ModalName>\PROMPT.md
C:\UE\T66\UI\Reference\PROMPT_INDEX.md
C:\UE\T66\SourceAssets\UI\Reference\Screens\<ScreenName>\
C:\UE\T66\SourceAssets\UI\Reference\Modals\<ModalName>\
```

Use the prompt index to start a new Codex chat per target. Each target owns its runtime image assets. If an asset from Shared or another target is useful, duplicate it into the current target folder and route the target to that copy.

Generated image outputs should be reported as local links/paths, not embedded inline, unless the user explicitly asks for inline previews.

Every implementation pass must end with packaged visual comparison. Compile success alone is not completion.
