Verified the live code matches the draft's claims. The gate logic is sound.

Result: OK

## Summary
Codex's draft is an accurate, faithful report of a real implementation. The live code at `T66RunStateSubsystem_Combat.cpp:493-547` gates enemy ragdoll on `PercentAfterDamage`, routes sub-threshold enemy hits through `LaunchCharacter` knockback-only (returning before any `HeroPhysics` call, so no disable/input-suppression), and re-anchors throw alpha + duration scaling to the 50→99 window only for the enemy path while non-enemy sources keep their original 0-anchored curve via the `bEnemyDamageReaction` branches. This matches both the prompt and my independent answer. Compile, editor proof, and staged-exe proof were run with the expected `EnemyKnockbackOnly` / `EnemyRagdoll` log evidence. Codex can finalize.

## Suggested Answer Patch
Add one explicit line stating the boundary semantics, since the draft leaves it implicit: the gate is `PercentAfterDamage <= StartPercent` → knockback-only, i.e. a hit landing *exactly at* 50% still only knocks back; throw begins strictly above 50% (matches "once above 50%"). Worth surfacing in the final answer so the user isn't surprised by the off-by-one at the boundary.

## Issues To Fix
- None blocking. The implementation is correct and complete relative to scope.

## Question For User
None — the threshold and 50→99 scaling were specified; remaining choices were tuning defaults Codex legitimately picked.

## Evidence Or Verification Gaps
- The staged-readiness wrapper returned overall **FAIL** (`BUILD_CONFIG_UNSUPPORTED` on durable-save and lifecycle gates). Codex attributes this to the Development staged config, not this change. That attribution is plausible but unproven in the draft — Codex should state in the final answer that this FAIL is **pre-existing / config-inherent and reproduces on an unmodified build**, otherwise the "staged validation passed" claim rests on an asserted-but-unverified cause. The targeted staged-exe proof (exit 0, correct log lines) is the stronger evidence and does carry the gameplay claim.

## Notes
- Good handling of the independent-answer caveats: knockback-only avoids ragdoll/input-suppression, non-enemy 0-anchor scaling is preserved (no double-apply), and trap/lava/miasma/boss are explicitly excluded to match the literal "enemy" wording.
- Minor: `T66IsEnemyDamageReactionSource` also matches any delivery name starting with `"Enemy"` (`:114`). Harmless, but if any non-enemy system uses an `Enemy`-prefixed delivery tag it would be swept in — not worth changing, just worth Codex being aware.
