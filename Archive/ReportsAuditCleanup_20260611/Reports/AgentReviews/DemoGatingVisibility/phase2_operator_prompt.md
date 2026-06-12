# FullOperator Prompt: Demo Gating Visibility Phase 2

You are Claude Operator for `C:\UE\T66`; Codex is Validator/Finisher.

Use FullOperator mode only inside the approved Phase 2 scope from:

`Reports/AgentReviews/DemoGatingVisibility/codex_operator_approval_phase2.md`

Do not use native goal tools. Do not commit, push, tag, reset, clean, run broad
Git/LFS scans, or inspect Unreal binary asset folders. Do not change code,
config, assets, build outputs, or Mini/minigame runtime files.

Working task:
Operator: Claude
Validator: Codex
Scope: Phase 2 only. Create separate Markdown inventories for demo-gated
invisible content and deprecated content.
Stop condition: docs are written, anchored to current source/config, and a
completion packet is written.

Approved docs:
- `Demo/DEMO_GATED_INVISIBLE_CONTENT.md`
- `Demo/DEPRECATED_CONTENT.md`

User decision:
- Deprecated inventory may include centrally declared arcade/minigame deprecated
  items as documentation-only entries, but no deprecated code/runtime changes.

Doc requirements:
1. `DEMO_GATED_INVISIBLE_CONTENT.md`
   - Track content intentionally unavailable/invisible in the demo after Phase 1.
   - Include controlling seam for each item so restoring later is easy.
   - Clearly mark items moved to available by Phase 1 (drugs, diploma upgrades,
     Steam/secret achievements) as no longer demo-gated.
   - Do not list deprecated content here unless it is also demo-gated.
2. `DEPRECATED_CONTENT.md`
   - Track deprecated content separately from demo-gated content.
   - Include config/code seam and current status.
   - Include central arcade/minigame deprecated feature settings as
     documentation-only entries.
   - If broad source comments expose compatibility-only deprecated fields, list
     them in a separate compatibility-retained section so they are not confused
     with feature gates.

Verification:
- Focused source/config searches are enough; no compile or capture is needed for
  docs-only phase.
- Write a completion packet to:
  `Reports/AgentReviews/DemoGatingVisibility/phase2_completion_packet.md`

Completion packet must include:
- Outcome
- Docs changed
- Anchors used
- Confirmation that no code/config/asset changes were intentionally made
- Token ledger with Claude token count if exposed by helper manifest or
  otherwise `Unavailable`
- Caveats
