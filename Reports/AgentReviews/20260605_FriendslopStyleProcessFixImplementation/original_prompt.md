User request:

Ok go for it

Context:

The user approved implementing the process/tooling solutions proposed after the FriendslopStyle Main Menu pilot was judged not good enough. The prior agreed solution set:

1. Amend FriendslopStyle so runtime chrome quality comes from externally authored/cleaned transparent PNG plates where needed, not generic Unreal-reconstructed rubber atoms.
2. Add containment/fitting checks so rows and child controls cannot overflow parent panels while the verifier still reports PASS.
3. Add a blocking visual fidelity scorecard/holistic gate so `VerifyUIFidelity.py` PASS counts cannot stand in for actual visual fidelity.

Task contract:

```text
Working task:
Operator: Codex
Validator: Claude
Scope: implement the approved process/tooling fixes for FriendslopStyle fidelity assessment: authored-plate instruction amendment, containment/fitting checks, and a blocking visual scorecard gate. Treat this as process/tooling only, not redoing the Main Menu screen in this pass.
Stop condition: docs and verifier/checklist support are updated, verified with a focused smoke test, and Claude cross-review is clean.
```

Relevant repo evidence:

- `UI/UI_AGENTS.md` routes FriendslopStyle through `UI/FriendslopStyle/FRIENDSLOP_STYLE_IMPLEMENTATION_INSTRUCTIONS.md`.
- `UI/FriendslopStyle/FRIENDSLOP_STYLE_IMPLEMENTATION_INSTRUCTIONS.md` currently says the reference is a visual-direction target and runtime uses sliced raster chrome atoms rendered by Slate.
- `Scripts/VerifyUIFidelity.py` currently evaluates checklist items, draws a contact sheet, and exits on FAIL count only; it does not enforce holistic visual gates or containment-specific assertions.
- `UI/Instructions/UI_LAYOUT_AND_SIZING_INSTRUCTIONS.md` already says parent containment and no clipping are required, but current verifier/checklist support is weak.
- `UI/FriendslopStyle/Checklists/main_menu_checklist.md` has a plain-text Holistic Visual Gate section that `VerifyUIFidelity.py` does not evaluate.

Ask:

Give a repo-grounded independent implementation plan for this process/tooling pass. Focus on what files to edit and what verification should prove the pass. Do not implement; Codex is operator.
