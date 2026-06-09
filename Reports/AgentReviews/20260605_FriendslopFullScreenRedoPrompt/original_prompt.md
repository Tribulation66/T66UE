User request:

Ok no, it failed EVERYWHERE youre focusing only on the width but the we also need visual gates for quality, and it should result in 96 fails, but this chat has gotten to laggy, so I need you to basically give the prompt for the agent, give him full context and information of where stuff is, and then have him make another attempt at the full screen, to look like the reference image. For all image generation he should deploy a codex CLI. The next screen needs that the elements on the assembled screen look like the elements from the extracted elements image, and all the sizing and positioning is correct. Go ahead and produce the prompt so I can retire this chat

Task contract:

```text
Working task:
Operator: Codex
Validator: Claude
Scope: produce a handoff prompt for the next agent to redo the full FriendslopStyle Main Menu against the reference, with full context, paths, constraints, and the corrected quality/fidelity expectations. No implementation in this chat.
Stop condition: provide a usable prompt and artifact path for the next agent.
```

Ask for Claude:

Review the planned handoff prompt for omissions or dangerous ambiguity. The output should help the next agent redo the full screen, using Codex CLI workers for image generation, and should make clear that visual quality gates must cover every load-bearing element, not just layout/width containment.
