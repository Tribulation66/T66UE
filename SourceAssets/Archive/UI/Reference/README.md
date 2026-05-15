# Runtime Reference UI Assets

This is an archived copy of the old Reference UI asset notes. The active runtime root is `RuntimeDependencies/T66/UI/Reference`.

```text
SourceAssets/UI/Reference/Screens/<ScreenName>/
SourceAssets/UI/Reference/Modals/<ModalName>/
SourceAssets/UI/Reference/Shared/
```

`Screens` and `Modals` are the destination folders for accepted runtime art. `Shared` is a bootstrap and compatibility source only. When a screen or modal chat touches a target, duplicate any reused Shared asset into that target folder and route the target to its own copy.

The sanitized asset inventory is `asset_inventory.csv`.
