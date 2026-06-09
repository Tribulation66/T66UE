Operator draft:

I agree with the proposed reference-first process, with a required correction:

The reference image should be the visual direction target, not the runtime asset. We should generate a full-screen Friendslop-style mockup/reference first, freeze it after user approval, then build the screen and reusable UI element kit against that reference. This gives Codex and Claude something concrete to inspect and something objective to compare captures against.

However, because the user explicitly wants FriendslopUI as an alternative to the current FlatStyle direction, the process docs must clearly declare that `UI/FriendslopUI` is a separate style lane. The current flat docs ban generated raster chrome; that remains true for FlatStyle work, but FriendslopUI would be a new approved textured/sliced UI style if the user signs off. Without that declaration, future agents will keep applying the flat chrome ban and block the very thing the user is trying to test.

Recommended process:

1. Generate 2-3 full-screen visual reference candidates with imagegen for one target screen, likely Main Menu.
2. Pick/freeze one reference after user approval. Store it as reference art, not runtime UI.
3. Run a style extraction pass: identify shape language, material language, palette, edge treatment, shadows, type direction, icon style, spacing density, and interactive state vocabulary.
4. Decompose the reference into a reusable element list: button families, panel frames, slots, tabs, badges, dividers, progress bars, modal shells.
5. Decide for each element whether it is code-native, fixed raster, 3-slice, 9-slice, tiled texture, or bespoke.
6. Generate/author individual blank UI plates only after the reference and element list are approved. Do not crop and ship the full-screen mockup as runtime chrome.
7. Keep all labels, player data, prices, scores, and localized text live.
8. Adapt the existing UI fidelity loop as the verification backbone: geometry extraction, screenshot/capture comparison, dump-based structural checks, pass log, accepted deltas.

Key guardrails:

- Reference image is a target, not an asset.
- Full-screen imagegen output cannot be wired directly into runtime UI.
- Individual UI plates must be blank chrome and must have explicit scaling rules.
- 9-slice will not fix bad generated art; the plate must be authored for slicing.
- The reference must go through a design approval gate before implementation.
- The process must define what is objective: geometry, hierarchy, color/material tokens, slice margins, state variants, and screenshot deltas.
- The process must define what is subjective: atmosphere, goofy/cozy feel, and final art taste. Those need user approval gates.

So: yes, use imagegen to establish visual direction, but the process should be "reference -> extraction -> asset kit -> implementation -> fidelity loop," not "generate a screen and copy it until it looks close."
