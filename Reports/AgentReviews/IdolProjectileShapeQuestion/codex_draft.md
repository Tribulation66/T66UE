Codex draft answer for cross-review:

Short answer: no, not if "projectile shapes" means the outgoing traveler carrier that flies from source to target.

Current live-code evidence:
- `Source/T66/Gameplay/T66OutgoingTravelerPoolSubsystem.cpp` maps all 20 idol traveler profile IDs to four mesh slots by category:
  - AOE -> slot 0
  - Pierce -> slot 1
  - DOT -> slot 2
  - Bounce -> slot 3
- That means there are 20 profile IDs and element colors, but only four outgoing projectile carrier shape families.
- `Source/T66/Gameplay/T66CombatVFX.cpp` does have per-element/per-category primitive activation placeholder builders. Those are visually different arrangements at activation/impact, such as Fire AOE explosion ring, Ice AOE frost disc/ring, Electricity AOE vertical strikes, Nature AOE branch spokes, and Wind AOE tornado stacks.
- However, several category families still share a similar base primitive language, especially Bounce and DOT, and the current implementation has not been visually signed off as 20 distinct silhouettes.
- No Weapon is separate from idol travelers. It routes through `HeroSingleTarget` and uses a white sphere single-target projectile.

Conclusion:
The correct user-facing answer should say: we currently have 20 idol visual profile IDs and 20 activation concepts, but not 20 distinct outgoing projectile shapes. If the requirement is that each idol's projectile silhouette is visibly unique before impact, this is a gap and the traveler profile mapping needs to be expanded beyond the four category slots.
