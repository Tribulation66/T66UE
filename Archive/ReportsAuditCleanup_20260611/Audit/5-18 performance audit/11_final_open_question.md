# Final Open Question - What Claude Should Know Before Planning

## Short Answer

The current repo is ready for source triage, but not ready for a trustworthy performance baseline. The biggest missing piece is measurement: there are no current Unreal Insights traces, no `stat unit/gpu/slate/memory` captures, no Deck 1280x800 screenshot pass, and no Mac/Linux cook/build proof.

## Most Important Performance Gaps

1. Game-thread work is likely the first CPU baseline target.
   - Many core actors tick directly.
   - Enemy movement is per-enemy tick.
   - Hero/enemy/combat/UI scopes use the custom lag tracker but do not yet have current Insights evidence.

2. Runtime generation and sync loading need profiling.
   - Seven-floor stages are generated inside the active map.
   - No runtime level streaming was found.
   - Character visual/VAT/world setup paths still have sync loads.

3. Rendering is partly optimized but not fully batched.
   - Tower terrain and some gameplay surfaces use HISM/ISM.
   - Enemies, heroes, interactables, projectiles, and many props still use individual components.
   - Some props have only one LOD and Nanite disabled.

4. UI is a real performance and platform risk.
   - No global invalidation rollout was found.
   - Frontend root has a retainer, but gameplay HUD still needs profiling.
   - Many screens still use 1920x1080 reference layouts.
   - Controller focus is not centrally guaranteed.

5. VFX needs runtime proof.
   - VFX budget CVars and quality tiers exist.
   - Niagara system internals and live GPU/CPU cost were not extractable statically.

## Most Important Steam Deck / Linux Gaps

1. `SocketSubsystemSteamIP` is configured as the Steam net driver but the UE 5.7 plugin descriptor only allows Win64 and Mac.
   - `OnlineSubsystemSteam` supports Linux.
   - `SocketSubsystemSteamIP` does not advertise Linux support.
   - This should be resolved before Linux/Deck build work goes deep.

2. Deck runtime profile is present but only partly applied.
   - Detection exists.
   - Media Viewer gating exists.
   - Deck UI scale, scalability, and FPS cap defaults are configured, but automatic application was not proven from static call-site checks.

3. Steam Input is absent.
   - Gamepad mappings exist.
   - No Steam Input manifest/API references were found.

4. Deck UI validation is missing.
   - 1280x800 screenshot and controller-only navigation are not proven.
   - Fixed 1920 UI roots still exist.

## Most Important macOS Gaps

1. Mac config exists but no Mac build artifact exists.
2. Metal SM6 shader compatibility is unproven.
3. Steam and Electra plugin posture looks better for Mac than Linux, but actual cook/build/runtime behavior is unverified.
4. Case-sensitive asset path behavior still needs cook validation even though source/config string checks did not find a clear mismatch.

## Repo State Warning

The working tree was already very dirty before these reports were created:

- 1,403 tracked status lines.
- Heavy changes in `Content`, `Source`, `SourceAssets`, `ToonStyle`, and `Model Generation`.

Claude should not treat any performance number captured from this state as a stable baseline unless the intended asset/source wave is intentionally part of the baseline.

## Recommended First Baseline

Before any remediation plan, capture these on the staged standalone build:

- `stat unit`
- `stat game`
- `stat gpu`
- `stat rhi`
- `stat scenerendering`
- `stat slate`
- `stat memory`
- `stat gc`
- Unreal Insights with `-trace=cpu,gpu,frame,bookmark`
- `ProfileGPU`
- `T66.Perf.Dump`

Suggested scenes:

- Frontend idle.
- Settings screen.
- Hero selection.
- Gameplay level just loaded.
- Normal enemy wave.
- Heavy VFX/boss/trap scene.
- Stage/floor transition.
- One representative minigame battle.
- 1280x800 Deck-style launch.

## Bottom Line

The likely first remediation lanes are:

- Capture real baseline first.
- Fix Linux/Deck Steam net-driver posture.
- Apply Deck runtime defaults beyond Media Viewer gating.
- Add or enforce UI invalidation/controller-focus/1280x800 validation.
- Profile enemy tick, terrain generation, sync loads, HUD refresh, and VFX budgets before changing behavior.

