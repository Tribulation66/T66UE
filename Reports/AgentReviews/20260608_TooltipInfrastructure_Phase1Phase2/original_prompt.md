User request:

Ok go for it build the infrastructure and do phase 1 and phase 2 in the next pass

Task contract:

Working task:
Operator: Codex
Validator: Claude
Scope: implement tooltip infrastructure plus Phase 1 and Phase 2: shared payload/presentation helpers, metadata/dump support, current helper consolidation, and pilot coverage for stats, HUD inventory/idol slots, flat info icons, items, powerups, and vendor.
Stop condition: code is edited, Claude reviews the plan/patch, and current compile/verification is attempted and reported.

Repo rules:

- Codex is Operator and Claude is Validator from .t66/operator-state.json.
- Use the UI folder router and UI instructions.
- Do not use native goal tools.
- Do not revert unrelated dirty worktree changes.
- Keep edits scoped to tooltip/UI infrastructure and the Phase 1/Phase 2 pilot surfaces.
- Claude validator must be read-only and use local Claude CLI, with ANTHROPIC_API_KEY absent in Process/User/Machine.
