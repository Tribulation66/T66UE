# Decision Block: Pierce/Bounce/DOT Idol Impact Presentation

## Status

NEEDS_HUMAN_DECISION before implementation.

## Decision

When a Pierce, Bounce, or DOT idol is triggered by the AOE weapon's official impact point, should the first structural proof apply the idol's category-native damage behavior, or should it only spawn category-native placeholder visuals while preserving the existing legacy idol damage path?

## Option A: Full Category-Native Idol Behavior

- Pierce idol publishes its own `IdolModifier` impact context and applies line/lane-style idol damage from the AOE impact point.
- Bounce idol publishes its own context and applies chained idol damage starting from the AOE impact point.
- DOT idol publishes its own context and applies idol-owned DOT to the hit target from the AOE impact point.
- This is the cleaner long-term combat model, but it is a larger behavior change and needs stronger damage-source and neutral-control proof.

## Option B: Presentation-First Placeholder

- Pierce/Bounce/DOT idols publish idol-owned contexts and spawn category-native placeholder visuals from the AOE weapon impact point.
- Damage remains on the current legacy idol damage path for this proof pass.
- This is smaller and faster, but it leaves a known temporary mismatch between the new impact-context presentation structure and the old damage behavior.

## Recommendation

Option A, unless the immediate priority is only proving visual/event routing. The user's previous direction was that idols should have their own damage source and their own impact point, so category-native idol behavior is the more consistent structure.
