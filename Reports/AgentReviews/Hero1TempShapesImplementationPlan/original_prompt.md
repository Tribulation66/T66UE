Original user request:

Yes correct all the hero 1 weapons are AOE, but they should have different AOE, shapes and patters. I do want to change the electricity to purple, and make sure we use ice blue for ice. Also we should get rid of the legacy ids and normalize ids first for sure. I agree with your other suggestions create an implementation plan first. And then we will go one by one on the shapes.

Task contract:

Working task:
Operator: Codex
Validator: Claude
Scope: planning-only for temporary Hero 1 AOE weapon shapes and idol visual variants, including purple Electricity, ice-blue Ice, canonical idol ID normalization, and removal of legacy-ID dependence.
Stop condition: deliver an implementation plan; no code/content changes beyond required review artifacts.

Relevant repo rules:

- Root router is C:\UE\T66\AGENTS.md.
- Current operator state is Codex operator, Claude validator.
- Mini/minigame scope is excluded.
- This is planning-only; do not implement.
- Combat work is owned by Gameplay and must respect Gameplay/GAMEPLAY_AGENTS.md and existing combat VFX contracts.
- Temporary simple shapes are infrastructure placeholders only. Final Niagara/material VFX remains governed by PPF, artifact parity, mechanism manifest, impact-context proof, visual/damage alignment, Unreal-owned capture, and staged verification when runtime/playable content changes.

Live facts from prior repo read to preserve:

- Hero 1 has four weapon rows: Hero_1_black_aoe, Hero_1_red_aoe, Hero_1_yellow_aoe, Hero_1_white_aoe. All are Branch=AOE.
- Idols have 16 canonical rows: Fire/Ice/Electricity/Nature x DOT/AOE/Pierce/Bounce, with four rarity variants per row conceptually needed for temporary visual shapes.
- Current outgoing traveler visual slots cover 16 element/category profiles but no rarity dimension.
- Current temporary projectile helper has four hero profiles and one idol overlay profile, not enough for the requested matrix.
- Current color helpers use yellow for Electricity; user wants purple. User wants ice blue for Ice.
- Legacy idol aliases should not be allowed as separate temp-visual keys; normalize IDs before lookup.

Requested Validator output:

- Review the implementation plan for scope, missing steps, risky assumptions, and repo-process conflicts.
- Keep output concise and end with Result: OK or Result: NEEDS_USER.
