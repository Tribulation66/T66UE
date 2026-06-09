Result: OK

## Independent Answer

The request is a defaults/CVar tuning change and the two models can implement it. The controlling duration path is `UT66KnockbackComponent` (`Source\T66\Gameplay\T66KnockbackComponent.cpp/.h`), but there are **two parallel sources of the same four timing levers**, and both must be patched in lockstep or the standalone won't reflect the change:

1. **Component `DefaultProfile`** (`T66KnockbackComponent.h:72–84`): `MinIncapacitationSeconds=1.65`, `MaxRagdollSeconds=5.25`, `SettleSpeed=165`, `SettleHoldSeconds=0.35`, `RecoveryBlendOutSeconds=0.35`. Used by the generic `ApplyKnockbackLaunch` path.
2. **TestRoom wipeout-arm CVar defaults** (`T66GameMode_TestRoom.cpp:116–144`): identical numbers (`1.65/5.25/165/0.35/0.35`). The TestRoom wipeout arm builds an override profile from these CVars (`MakeWipeoutArmKnockbackProfile`, lines 446–454) and passes it explicitly, so it **bypasses `DefaultProfile` entirely**. If the playable standalone's wipeout is the TestRoom arm, editing only the `.h` defaults will have *no visible effect* — Codex must update the CVar defaults too.

No `.ini` overrides these CVars (checked `Config\*.ini`), so the in-code defaults are authoritative.

**How the timing actually composes** (skeletal ragdoll path, `UpdateActiveKnockback`/`UpdateRecovery`):
- Control can only return after `RestoreFromKnockback()`, which runs at the end of the *Recovering* blend.
- The settle timer (`LowVelocityStartedTimeSeconds`) can't even start until `Now >= ControlRestoreTimeSeconds` (i.e. past `MinIncap`) — line 585–593.
- So the floor on total incap ≈ `MinIncap + SettleHold + BlendOut` ≈ `1.65 + 0.35 + 0.35 = 2.35s` today (capped/forced at `MaxRagdoll + blend`).

The "~0.5s after contact" the user wants is the window **from the body coming to rest to control return**, which equals `SettleHoldSeconds + RecoveryBlendOutSeconds`. To hit ~0.5s *and* not have `MinIncap` dominate, all of these need to move together. Suggested starting values:
- `MinIncapacitationSeconds → ~0.15` (settle-speed gating, not a long timer, keeps them from standing mid-flight; respect TestRoom clamp floor of 0.15).
- `SettleHoldSeconds → ~0.15`, `RecoveryBlendOutSeconds → ~0.35` → ~0.5s post-contact, blend stays smooth.
- `MaxRagdollSeconds → ~1.5–2.0` (forced-recovery safety ceiling; must stay > MinIncap+0.1) so a hero that never fully settles still recovers fast.
- Leave `SettleSpeed=165` unless settle detection proves too strict in playtest.

Then run the focused compile and staged standalone refresh per the stop condition, and confirm by feel.

## Evidence Checked
- `T66KnockbackComponent.h:72–84,196` — `DefaultProfile` timing fields + clamps.
- `T66KnockbackComponent.cpp:134,179–182,492,585–616,633–647` — profile selection, timer setup, Active→Recovering→Restore state machine.
- `T66GameMode_TestRoom.cpp:116–144,438–477,513–518` — wipeout-arm CVar defaults and the override profile that bypasses `DefaultProfile`.
- `Config\*.ini` — no CVar overrides for these keys.
- No console-CVar layer inside the component itself (only `ProfileOverride`/`DefaultProfile`).

## Questions Or Blockers
None that require the user. One interpretation Codex should state explicitly rather than ask: "0.5s after contact" is read as the post-settle window (`SettleHold + BlendOut`), not total airborne+settle time.

## Caveats
- **Two-source risk is the main catch**: patch both the `.h` `DefaultProfile` and the five TestRoom CVar defaults, or verify which path the playable standalone wipeout actually uses. A `.h`-only edit will likely look like "nothing changed."
- Exact split of the ~0.5s between `SettleHold` and `BlendOut`, and the `MinIncap`/`MaxRagdoll` floors, are feel calls — flag for standalone playtest tuning; the numbers above are a starting point, not final.
- Lowering `MinIncap` leans recovery entirely on settle-speed detection; if a hero is still sliding/bouncing when the gate opens, recovery waits for the body to drop under `SettleSpeed`, so very bouncy launches may still feel longer than 0.5s — watch for this in testing.
- If a hero Blueprint sets `DefaultProfile` per-instance (EditDefaultsOnly), that would shadow the C++ default for the generic path; couldn't inspect `.uasset` BPs read-only, so verify in-editor that the hero BP doesn't override these fields.
