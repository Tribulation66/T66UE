# FriendslopStyle Main Menu Pass14 Decision Block

Task: continue pass14 after replacing freeform generated UI plates with a
reference-first extraction and component-gate process.

Current decision gate:

The corrected reference-first gate has been built and run. Exact reference crops
and content masks were produced under:

`C:\UE\T66\Saved\Codex\UI\FriendslopStyle\MainMenu\pass14_reference_component_gate\`

The first account-backed built-in imagegen candidates failed the component gate:

`C:\UE\T66\Saved\Codex\UI\FriendslopStyle\MainMenu\pass14_reference_component_gate\pass14_candidate_component_gate_report.md`

Blocking reason:

- Built-in imagegen can create blank candidates, but without true mask control
  it regenerates the component material and silhouette instead of preserving the
  exact reference crop.
- Shipping direct cropped/masked reference pixels as runtime plates remains
  disallowed by the existing pass14 handoff unless the user explicitly changes
  that rule.
- CLI/API mask inpainting would provide direct file/mask control, but it is
  explicitly forbidden by the pass14 handoff unless the user explicitly changes
  that rule.

Decision needed before continuing runtime wiring:

1. **Allow direct reference-derived runtime plates with strict gates.**
   Codex can use the exact reference crop as the source, remove only masked live
   content zones locally or with approved edits, and ship the result only if the
   component gate proves no smudges, no content remnants, and close silhouette
   and material parity. This changes the previous "do not ship crop/masked
   plates" boundary.

2. **Allow a true mask-inpaint tool path.**
   Codex can use a tool that accepts a local image and explicit content mask,
   then run the same component gate before runtime wiring. The available
   built-in imagegen path does not expose this mask parameter; CLI/API fallback
   would require explicit user approval and conflicts with the current no
   `OPENAI_API_KEY` rule.

3. **Keep built-in-only regeneration and relax exactness.**
   Codex can keep iterating account-backed built-in prompts, but the current
   gate evidence shows this produces nice-looking but wrong chrome. This is not
   recommended for the requested "exactly like the reference" goal.

Recommended choice:

Choose option 1 if the priority is exact visual fidelity inside the current
no-API/no-CLI constraint. Choose option 2 if the "no crop/masked runtime plate"
rule must remain strict and exactness still matters. Do not choose option 3
unless approximate Friendslop-style chrome is acceptable.
