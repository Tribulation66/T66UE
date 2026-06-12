Verdict: REVISE

## Blockers

- **Gameplay-semantics change needs explicit user confirmation, not interpretation.** The packet's own Codex Position section flags this and asks Claude to check. Moving from a filled 180° sector to a crescent-band hit shape changes what counts as a hit for the Hero 1 AOE — that is a balance/design decision, not a visual fix. The user said "hitbox matches the visual with the debug line around it," which is ambiguous: it could mean (a) tighten the outer ring + draw the inner ring on debug only, or (b) actually carve out the inner hole as un-hittable space. Resolve via a one-question AskUserQuestion to the user before any code change. Until that lands, the plan is not safe to greenlight at the AGENTS.md go-ahead gate.

## Major Issues

- **Inner-radius ratio value is undefined.** The plan promises a `FWeaponData` inner-radius ratio field but never states the planned value. Live findings give inner ~222 cm / outer-carrier ~413 cm → ratio ≈ 0.537. State the exact ratio that will be committed and where it was measured (commandlet output line, frame, or asset).
- **`BaseVisualRadius` change is left ambiguous.** Live findings: CSV is `435.0`, carrier is ~`413` cm. Step says "update if needed" — decide now. If you change it, downstream item-scale math and any other binding rows that reference it must be re-validated; if you don't, document why the ~22 cm mismatch is acceptable.
- **Weapon row identifier is informal.** "Hero 1 black AOE row" is not a primary key. Name the exact `RowName`/RowID in `Weapons.csv` so the diff is reviewable.
- **Proof target positions not enumerated.** Verification step 5 names four case classes (inside-band, inner-hole, outside-angle, outside-radius) but the actual local-space coordinates and expected PASS/FAIL outcomes are not in the packet. Without those, the anti-lookalike discriminator cannot be evaluated from the proof bundle alone.
- **LFS/`.uasset` surface is hand-waved.** Importing `DT_Weapons.uasset` and (optionally) `DT_CombatVFXBindings.uasset` re-touches LFS assets. "Verify narrow paths only" is a posture, not a procedure — name the exact `.uasset` paths expected to change and the git/LFS check that confirms nothing else moved.
- **`T66Hero1AxeAOEVFXCommandlet.cpp` not in edit scope but referenced as the source of measurements.** If the inner-radius ratio is derived from commandlet output, confirm whether the commandlet itself needs a recalibration pass or assertion update so future agents don't drift the carrier without updating the data contract.

## Minor Issues

- `T66PlayerController_Overlays.cpp` touch crosses from combat into overlays/automation — call out that this is the proof-target source and confirm no overlay runtime behavior changes.
- "Combat-owned crescent-band geometry metadata" should be named one canonical way across `MASTER_COMBAT.md`, `CombatVFXInfrastructureInventory.md`, and `Hero1AxeAOESlashMechanismPacket.md`. Pick one term ("inner-radius ratio" or "crescent inner ratio") and use it everywhere.
- Handoff prompt under `Reports/Proof/CombatVFX/Hero1AxeAOE_HitboxCleanup_20260528/` is mentioned but its scope/contents are not previewed. List the bullets the prompt will hand to the next agent (item-stat scale confirmation + idol overlay design) so the user can sanity-check the handoff before it ships.
- No mention of whether `pending_issues_Combat.md` had an entry assuming a filled sector — if so, it needs an update or a removal note.
- Rollback says "restore the Hero 1 AOE row's inner-radius ratio to `0`" — confirm that `0` is the schema's documented "filled sector" sentinel and that the validator will not flag `0` as missing data.

## Clarifying Questions

1. Does the user want the inner hole to be un-hittable (gameplay semantics change), or only debug-drawn while damage stays as a filled sector?
2. Should `BaseVisualRadius` be tied automatically to the commandlet-measured carrier (single source of truth) or remain a manually-set CSV value?
3. Is the next-agent handoff one prompt covering both item/stat confirmation and idol overlay, or two separate prompts?

## Required Verification

The packet's verification list is mostly right but needs to be made concrete:

- Specify the exact proof target coordinates (local to slash origin) and expected hit/miss for each of the four case classes.
- Include a debug-frame requirement showing both outer and inner sector boundary lines visible around the crescent, captured via the existing `Scripts/CaptureT66GameplayVideo.ps1` / `RunHero1AxeAOEVFXBindingProof.ps1` flow (no new capture tooling).
- Add an explicit `git status --short` + LFS pointer check before/after data-table reload, scoped to the named `.uasset` paths only.
- Add a focused `T66Editor Win64 Development` build line (already listed) plus a confirmation that no other gameplay module rebuilt.
- Add a validator pass log line showing the new geometry-contract guard fired.

## Rationale

The plan correctly identifies that the dormant `InnerRadius` path in `BuildSlashTargets` is the right hook and avoids the cheapest-wrong-result trap called out in the anti-lookalike. PPF/Artifact-Parity/Mechanism Manifest sections are present and substantive. However, Codex itself surfaces a real design ambiguity — crescent-band vs filled sector is a gameplay change, not a visual one — and the packet still has several concrete unknowns (ratio value, BaseVisualRadius decision, exact row name, proof coordinates) that should be resolved before the user is asked for the AGENTS.md go-ahead. Fix those and the plan is ready to present.

