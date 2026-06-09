User request:

Can we add an icon in the minimap wherever there is a trap, it can just be a red circle.

Task contract:

Working task:
Operator: Codex
Validator: Claude, if the local validator path is available
Scope: add a minimap marker for trap actors, using a simple red circle; keep the change scoped to HUD/minimap and trap marker data.
Stop condition: implementation is in place, focused build/runtime verification is attempted, and exact proof or blockers are reported.

Relevant repo rules:

- Follow AGENTS.md task contract and Operator/Validator protocol.
- Current operator state says Codex operator, Claude validator.
- Runtime-facing gameplay/UI changes need focused compile/build verification; staged standalone validation when they affect playable standalone.
- UI router owns HUD/minimap Slate code; do not run full UAT for a one-screen change unless required.
- World map reference says tower minimap/full-map should be active-floor views.
- Pending issues were checked for UI, Gameplay, and GameMode.
