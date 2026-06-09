# Original Prompt

User wants to start Hero 1 weapon placeholder visuals by turning four Paint sketches plus written descriptions into professional imagegen visual targets. The user clarified that T66 has a Codex imagegen wrapper process that does not require an API key. This turn should produce the first-pass imagegen visual targets before Unreal implementation.

## Task Contract

Working task: Hero 1 four-weapon placeholder imagegen planning and readiness check.
Operator: Codex.
Validator: Claude if available through the local subscription-backed helper.
Scope: Non-Mini. Generate first-pass visual targets for Hero 1 black/red/yellow/white AOE weapons using the user's sketches and descriptions. No Unreal asset edits, DataTable edits, staged build, or VFX implementation in this turn unless the user explicitly proceeds after approving a visual target.
Stop condition: save the generated visual targets to a durable project path, report prompt/process, and call out gameplay/data mismatches against live repo state.

## User-Specified Visual Targets

- Black rarity: one black lunar crescent / half-moon slash, same general shape as current Hero 1 black AOE. Primary point at the center of the slash does 100% weapon damage and triggers idols. The rest of the slash body does 50% damage and does not trigger idols.
- Red rarity: three connected smaller half-circle slashes. Overall projectile about 50% wider than black. Base weapon damage 120% of black. Three impact points, each does 100% weapon damage and triggers idols. Non-impact body does 50% damage.
- Yellow rarity: five connected smaller half-circle slashes. Overall projectile about 150% the black size. Base weapon damage 150% of black. Five impact points, each does 100% weapon damage and triggers idols. Non-impact body does 50% damage.
- White rarity: one large slash about twice black size. Base weapon damage 200% of black. Whole projectile is primary damage and impact point, so any enemy touched takes 100% and triggers idols.

## Relevant Repo Rules

- Follow `AGENTS.md`, `.t66/operator-state.json`, `OPERATOR_VALIDATOR_PROTOCOL.md`, `Gameplay/GAMEPLAY_AGENTS.md`, and `Gameplay/Combat/CombatVFXAuthoringProcedure.md`.
- Combat VFX visual targets are process-governed. Imagegen mockups are visual direction only and cannot replace Niagara/material implementation or Unreal-owned capture proof.
- For T66 imagegen mockups, `AGENTS.md` and `Gameplay/Combat/CombatVFXAuthoringProcedure.md` say to use the approved account-backed imagegen path and not require or revive `OPENAI_API_KEY` API scripts.

## Current Live Repo Observations For Validator

- `Content/Data/Weapons.csv` already has Hero 1 black/red/yellow/white AOE rows.
- Current Hero 1 damage multipliers are black `1.20`, red `1.35`, yellow `1.55`, white `1.80`; these do not exactly match the requested relative base multipliers of 100/120/150/200.
- Current projectile counts/patterns are black `1`/Single, red `2`/TwinFan, yellow `1`/Single, white `1`/Single; these do not match the requested 1/3/5/all-contact impact rules.
- `Content/Data/CombatVFXBindings.csv` maps all four Hero 1 AOE rarity rows to the same Hero 1 AOE mesh slash Niagara system currently.
- User corrected the process gate: proceed through the no-API-key account-backed Codex imagegen wrapper.
