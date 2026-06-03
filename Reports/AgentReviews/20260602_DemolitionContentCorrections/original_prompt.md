EXECUTE — plan approved. Proceed with the phased plan and sub-agent split for T66/Chadpocalypse.

Working task:
Operator: Codex
Validator: Claude Code
Scope: implement the approved demolition/content-correction pass for C:\UE\T66. Demote T66Mini/T66TD/T66Idle/T66Deck into main-module shell classes with SHELVED READMEs, add T66Buried shell, reduce Versus/arcade to shelved shell, centralize minigame/Versus/DailyDescent gating, shelve all Daily Descent UI/entry/backend/save surfaces without deleting them, replace casino games with Coin Flip / Guess the Cup / Pick Longest or Shortest Stick / Find the Joker, implement 4 independent shop buy slots with explicit rarity weights, keep vendor guaranteed and casino chance-based, remove dead per-difficulty base rarity fields/getters, replace direct companion unlock with boss caged companion free/interact unlock, add deterministic proof hooks, then run clean full build + cook/stage and staged smoke.
Stop condition: no git operations. Ignore Saved/HygieneBackups lookalikes. Do not purge Daily Descent backend/save fields. Delete old casino enum strings/save/backend references; no old-save compatibility needed.

User-resolved details:
1. Daily Descent: shelve everything, delete nothing. Route UI/entry through central shelved gate and make backend/game-instance/run-save surfaces inert under that gate.
2. Old casino enum strings: delete RPS/BlackJack/Lottery/Plinko/BoxOpening enum strings and save/backend references.
3. Shop: exactly 4 slots for buy/sell/buyback. Buy slots always show an item. Every buy slot independently rolls rarity with explicit named tunables: Black 70%, Red 25%, Yellow 4.5%, White 0.5%.

Approved sequencing:
- Central gate and main-module shells compile first.
- Rewire all entry points to the gate.
- Delete side-module/content/source-asset/runtime roots only after routes compile against shells.
- Lead owns shared serialized files, especially T66GameMode_WorldInteractables.cpp, T66WidgetGameRegistry.cpp, frontend/gating routes, config/build roots, and arcade teardown core.

Relevant repo rules:
- Follow C:\UE\T66\AGENTS.md and OPERATOR_VALIDATOR_PROTOCOL.md.
- Codex is Operator, Claude is Validator.
- This is implementation/proof work, so current verification is required; prior evidence cannot replace requested build/stage/smoke.
- No git operations.
- Preserve user/peer changes; do not revert unrelated changes.
- Report phase gates and exact verification.

Claude task:
Produce a read-only independent implementation risk map and recommended checkpoints. Focus on missed files, sequencing hazards, compile blockers, and proof-hook locations. Do not mutate files.
