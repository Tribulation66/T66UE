# Pending Issues - T66Mini

## T66Mini Build Script References Missing UI Components Directory

- Severity tag: [Minor]
- What's wrong: The final cleanup stage build logged `Source/T66Mini/T66Mini.Build.cs: warning: Referenced directory 'C:\UE\T66\Source\T66Mini\Public\UI\Components' does not exist.` The build still succeeds, but the module rules reference a folder that is absent from the current source tree.
- Why it's out of scope now: This pass is scoped to gameplay visual cleanup, archive deletion, old import script retirement, and staged verification. The warning belongs to the T66Mini module cleanup surface.
- What fixing it would entail: Inspect `Source/T66Mini/T66Mini.Build.cs`, remove or recreate the missing include directory reference as appropriate, then run a focused build to confirm the warning is gone.
