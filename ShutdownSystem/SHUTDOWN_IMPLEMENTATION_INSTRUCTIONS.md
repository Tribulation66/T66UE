# Shutdown Implementation Instructions

Use these instructions when adding or updating T66 shutdown participants.

## Participant Checklist

1. Identify the owner that already owns the resource.
2. Add a private shutdown helper on that owner, for example `ShutdownRuntimeResources`.
3. Call that helper from both the shutdown participant and normal `Deinitialize` / `EndPlay` where relevant.
4. Make the helper idempotent.
5. Register with `UT66ShutdownSubsystem` during owner initialization.
6. Pick the earliest safe phase.
7. Give the participant a timeout budget and log slow work.
8. Add or update the owner row in `SHUTDOWN_REGISTRY.md`.

## Preferred Pattern

```cpp
ShutdownParticipantHandle = Shutdown->RegisterParticipant(
    this,
    TEXT("Owner.Resource"),
    ET66ShutdownPhase::AsyncWork,
    100,
    1.0,
    false,
    FT66ShutdownParticipantDelegate::CreateUObject(this, &ThisClass::HandleShutdown));
```

The participant callback should return `true` if it completed its bounded cleanup, and `false` if it had to skip or abandon required work.

## Anti-Patterns

- Calling another subsystem's private cleanup directly.
- Waiting forever for async work.
- Adding `CoUninitialize` outside the class that called `CoInitializeEx`.
- Treating the shutdown subsystem as a UObject garbage collector.
- Routing automated proof `RequestExitWithStatus` calls through player quit shutdown without preserving their status-code contract.
- Scheduling new async work after shutdown has started.

## Verification

At minimum:

- focused compile
- inspect shutdown logs for participant timing

For playable quit changes:

- staged standalone refresh
- shortcut target verification
- quit smoke from the affected surface

