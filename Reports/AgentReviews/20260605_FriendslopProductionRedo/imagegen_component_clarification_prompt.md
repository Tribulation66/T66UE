# Question-Only Prompt: Imagegen vs Component Rebuild Clarification

Working task:
Operator: Codex
Validator: Claude
Scope: clarify whether creating proper components implies regenerating imagegen assets, with no implementation.
Stop condition: separate the image asset generation step from the runtime component rebuild step and state the correct next method.

User asks:

Ok now youre getting confused because the way we create a proper button component was it not through imagegen? So it would mean regeneration of imagegen no?

Context:

- Prior answer said "rebuild elements" does not mean generating more elements through imagegen.
- User correctly points out FriendslopStyle rubber button chrome was previously created through image generation.
- Need clarify that imagegen can be used to create clean blank chrome/source art, but the component itself is native Slate/UMG: layout, text/icon ownership, states, sizing, bindings.
- Need distinguish valid regeneration from invalid screenshot-crop-and-inpaint.

Answer only. No implementation.
